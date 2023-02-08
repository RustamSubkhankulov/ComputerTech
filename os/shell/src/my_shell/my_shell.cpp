#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>

//---------------------------------------------------------

#include "../../inc/my_shell/my_shell.h"
#include "../../inc/parser/parser.h"

//=========================================================

static int execute_pipeline(const Cmnds* cmnds);

static void write_promt();

static int set_sig_handler();

static void sigint_handler(int signo);

//=========================================================

enum Exec_pipe_ret_val
{
    NORMAL          = 0,
    TERMINATE_SHELL = 101,
};

//=========================================================

int my_shell(const int argc, const char** argv)
{
    assert(argv);

    int err = set_sig_handler();
    if (err != 0)
        return err;

    while(1)
    {
        write_promt();

        struct Cmnds cmnds = {0};
        
        err = cmnds_ctor(&cmnds);
        if (err < 0)
        {
            fprintf(stderr, "cmnds_ctor() failed \n");
            return err;
        }

        err = parse_args(&cmnds, 0);
        if (err < 0)
        {
            fprintf(stderr, "parse_args() failed \n");
            return err;
        }

        int term_code = execute_pipeline(&cmnds);
        if (term_code != NORMAL && term_code != TERMINATE_SHELL)
            return term_code;

        err = cmnds_dtor(&cmnds);
        if (err < 0)
        {
            fprintf(stderr, "cmnds_dtor() failed \n");
            return err;
        }

        if (term_code == TERMINATE_SHELL)
            break;   

        putchar('\n');

    }

    return 0;
}

//---------------------------------------------------------

static int set_sig_handler()
{
    sighandler_t old = signal(SIGINT, sigint_handler);
    if (old == SIG_ERR)
    {
        fprintf(stderr, "signal() failed: %s \n", strerror(errno));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static void sigint_handler(int signo)
{
    printf("\n" "My final message. Goodb ye \n");
    exit(0);
}

//---------------------------------------------------------

static void write_promt()
{
    printf(">");
    fflush(stdout);
}

//---------------------------------------------------------

static int execute_pipeline(const Cmnds* cmnds)
{
    assert(cmnds);

    if (cmnds->size != 0 &&  strcmp("exit", cmnds->data[0].prog_name) == 0)
        return TERMINATE_SHELL;   

    int pid = 0;
    int pipe_fd[2] = { 0 };

    int cmnds_num = cmnds->size;

    int previous_out_fd = 0;

    for (unsigned iter = 0; iter < cmnds_num; iter++)
    {
        pipe(pipe_fd);

        pid = fork();

        if (pid == 0)
        {
            if (iter != cmnds_num - 1)
            {
                int err = dup2(pipe_fd[1], 1);
                if (err == -1)
                {
                    fprintf(stderr, "dup2() syscall failed: %s \n", strerror(errno));
                    return -errno;
                }

                err = close(pipe_fd[1]);
                if (err == -1)
                {
                    fprintf(stderr, "close() syscall failed: %s \n", strerror(errno));
                    return -errno;
                }

                err = close(pipe_fd[0]);
                if (err == -1)
                {
                    fprintf(stderr, "close() syscall failed: %s \n", strerror(errno));
                    return -errno;
                }
            }

            if (iter != 0)
            {
                int err = dup2(previous_out_fd, 0);
                if (err == -1)
                {
                    fprintf(stderr, "dup2() syscall failed: %s \n", strerror(errno));
                    return -errno;
                }

                if (previous_out_fd != 0)
                {
                    err = close(previous_out_fd);
                    if (err == -1)
                    {
                        fprintf(stderr, "close() syscall failed: %s \n", strerror(errno));
                        return -errno;
                    }
                }
            }

            int err = execvp(cmnds->data[iter].prog_name, cmnds->data[iter].arg_vec.data);
            if (err == -1)
            {
                fprintf(stderr, "execvp() syscall failed: %s \n", strerror(errno));
                return -errno;
            }

            break;
        }
        else
        {
            previous_out_fd = pipe_fd[0];

            int err = close(pipe_fd[1]);
            if (err == -1)
            {
                fprintf(stderr, "close() syscall failed: %s \n", strerror(errno));
                return -errno;
            }
        }
    }

    if (pid != 0)
    {
        for (unsigned iter = 0; iter < cmnds_num; iter++)
        {
            pid_t term = wait(NULL);
            if (term == -1)
            {
                fprintf(stderr, "wait() syscall failed: %s \n", strerror(errno));
                return -errno;
            }
        }

        int err = close(previous_out_fd);
        if (err == -1)
        {
            fprintf(stderr, "close() syscall failed: %s \n", strerror(errno));
            return -errno;
        }
    }

    return 0;
}

//---------------------------------------------------------


