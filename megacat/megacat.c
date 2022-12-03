#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>              
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <poll.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

//=========================================================

#define INIT_POLLFD(pollfds, ind, fdescr, evnt, revnt)      \
                                                            \
    do                                                      \
    {                                                       \
        pollfds[ind].fd = fdescr;                           \
        pollfds[ind].events  = evnt;                        \
        pollfds[ind].revents = revnt;                       \
                                                            \
    } while (0);                                    
    

//=========================================================

struct Child_pipe
{
    pid_t pid;

    int fdin;
    int fdout;
};

//---------------------------------------------------------

static const size_t Buffer_size = 4096ul;

// static const struct timespec Inf_timeout = {.tv_sec = -1, .tv_nsec = 0};
static const int Inf_timeout = -1;

//=========================================================

static int execute_check(struct Child_pipe* child_pipes, unsigned prog_times);

static char** alloc_buffers(unsigned prog_times);

static void free_buffers(char** buffers, unsigned prog_times);

static int perform_io(struct Child_pipe* child_pipes, unsigned prog_times, 
                                   struct pollfd* pollfds, char** buffers);

static void setup_start_pollfds(struct Child_pipe* child_pipes, unsigned prog_times, 
                                                            struct pollfd* pollfds);

//=========================================================

int main(const int argc, const char** argv)
{
    const char* prog_name = argv[1];

    errno = 0;
    unsigned prog_times = (unsigned) atoi(argv[2]);
    
    // Array describing children processes and their pipes
    struct Child_pipe* child_pipes = (struct Child_pipe*) calloc(prog_times, sizeof(struct Child_pipe));
    assert(child_pipes);

    int pid = 0;
    int pipe_fd_in [2] = { 0 };
    int pipe_fd_out[2] = { 0 };

    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        int err = pipe(pipe_fd_in);
        assert(err != -1);

        err = pipe(pipe_fd_out);
        assert(err != -1);

        pid = fork();
        assert(pid != -1);

        if (pid == 0) // child
        {
            int input  = pipe_fd_in[0]; 
            int output = pipe_fd_out[1];

            err = dup2(input, 0);
            assert(err != -1);

            err = dup2(output, 1);
            assert(err != -1);

            // close pipes as we already reopened them using dup2
            for (unsigned ind = 0; ind < 2; ind++)
            {
                err = close(pipe_fd_in[ind]);
                assert(err == 0);

                err = close(pipe_fd_out[ind]);
                assert(err == 0);
            }

            // close all fd's inherited from parent
            for (unsigned ind = 0; ind < iter; ind++)
            {
                err = close(child_pipes[ind].fdin);
                assert(err == 0);

                err = close(child_pipes[ind].fdout);
                assert(err == 0);
            }

            // executing testing prog
            err = execvp(prog_name, &prog_name);
            assert(err != -1);

            return 0;
        }
        else if (pid != 0) // parent
        {
            // remember needed fd's
            child_pipes[iter].fdin  = pipe_fd_in [1];
            child_pipes[iter].fdout = pipe_fd_out[0];

            // close unessessary fd's
            err = close(pipe_fd_in[0]);
            assert(err == 0);

            err = close(pipe_fd_out[1]);
            assert(err == 0);
        }
    }

    int err = execute_check(child_pipes, prog_times);
    assert(err == 0);

    // wait every child to terminate
    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        err = wait(NULL);
        assert(err != -1);
    }

    free(child_pipes);

    return 0;
}

//---------------------------------------------------------

static char** alloc_buffers(unsigned prog_times)
{
    char** buffers = (char**) calloc(prog_times, sizeof(char*));
    assert(buffers);

    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        buffers[iter] = (char*) calloc(Buffer_size, sizeof(char));
        assert(buffers[iter]);
    }

    return buffers;
}

//---------------------------------------------------------

static void free_buffers(char** buffers, unsigned prog_times)
{
    assert(buffers);

    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        free(buffers[iter]);
    }

    free(buffers);
}

//---------------------------------------------------------

static int perform_io(struct Child_pipe* child_pipes, unsigned prog_times, 
                                   struct pollfd* pollfds, char** buffers)
{
    assert(child_pipes);
    assert(pollfds);
    assert(buffers);

    uint64_t  in_checksum = 0;
    uint64_t out_checksum = 0;

    nfds_t pollfdn = prog_times * 2 + 2;

    while(1)
    {
        int err = poll(pollfds, pollfdn, Inf_timeout);
        // routine
    }

    return (in_checksum == out_checksum);
}

//---------------------------------------------------------

// TODO просыпатсья только тогда, когда надо - убрать ненужные read write

static void setup_start_pollfds(struct Child_pipe* child_pipes, unsigned prog_times, struct pollfd* pollfds)
{
    assert(child_pipes);
    assert(pollfds);

    unsigned cur_pollfd = 0;

    INIT_POLLFD(pollfds, cur_pollfd, 0, POLLIN, 0);
    cur_pollfd++;

    for (unsigned iter = 1; iter < prog_times + 1; iter++)
    {
        INIT_POLLFD(pollfds, cur_pollfd, child_pipes[iter].fdin, POLLOUT, 0);
        cur_pollfd++;

        INIT_POLLFD(pollfds, cur_pollfd, child_pipes[iter].fdout, POLLIN, 0);
        cur_pollfd++;   
    }

    INIT_POLLFD(pollfds, cur_pollfd, 1, POLLOUT, 0);

    return;
}

//---------------------------------------------------------

static int execute_check(struct Child_pipe* child_pipes, unsigned prog_times)
{
    assert(child_pipes);

    struct pollfd* pollfds = (struct pollfd*) calloc(prog_times * 2 + 2, sizeof(struct pollfd));
    //                                               ^^^^^^^^^^^^^^^^^^
    //                                               2 (write-end and read-end) for each of pipes 
    //                                               and 2 for stdin and stdout of parent process

    assert(pollfds);

    char** buffers = alloc_buffers(prog_times);
    assert(buffers);

    int check_ok = perform_io(child_pipes, prog_times, pollfds, buffers);

    if (check_ok < 0)
    {
        fprintf(stderr, "Error occured duting check. \n");
        return -1;
    }
    else if (check_ok == 1)
    {
        fprintf(stderr, "Check passed successfully. \n");
    }
    else if (check_ok == 0)
    {
        fprintf(stderr, "Check failed successfully XD. \n");
    }

    free_buffers(buffers, prog_times);
    free(pollfds);

    return 0;
}

//---------------------------------------------------------




















































































































//---------------------------------------------------------

