#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>

// action, queue, wait, suspend, procmask, fillset,  pending

//=========================================================

#define ERR(action)                                         \
                                                            \
    do                                                      \
    {                                                       \
                                                            \
        fprintf(stderr, "File: %s ",  __FILE__);            \
        fprintf(stderr, "Line: %d\n", __LINE__);            \
        action;                                             \
                                                            \
    } while (0);                                
 
//=========================================================

static int sig_queue       (const pid_t pid, int signum, union sigval signal_value);

static int sigsend_snd     (const pid_t pid, const char val);

static int sigsend_snd_stop(const pid_t pid);

static void sigsend_handler(int signum, siginfo_t *info, void *ucontext);

static int sigsend_change_handler(void);

//---------------------------------------------------------

static int open_file(const char* filename);

static int close_file(int file_descr);

static ssize_t safe_write(int fd, const void* buf, size_t count);

//---------------------------------------------------------

static int cat(const int argc, const char** argv);

static int cat_write(void);

static int cat_read(const pid_t write_pid, const int argc, const char** argv); 

static int cat_read_send_file(const pid_t write_pid, const int fd);

//=========================================================

static const unsigned Children_num = 2;

//---------------------------------------------------------

// RT##i = SIGRTMIN + Real_time_signum
static const int Real_time_signum = 1;

static int End_of_data_flag = 0x1000;

//=========================================================

int main(const int argc, const char** argv)
{
    return cat(argc, argv);
}

//=========================================================

int cat(const int argc, const char** argv)
{
    assert(argv);

    pid_t write_pid = fork();
    if (write_pid == -1)
    {
        ERR(fprintf(stderr, "fork() failed: %s \n", strerror(errno)));
        return -errno;
    }
    else if (write_pid == 0)
    {
        return cat_write();
    }

    pid_t read_pid = fork();
    if (read_pid == -1)
    {
        ERR(fprintf(stderr, "fork() failed: %s \n", strerror(errno)));
        return -errno;
    }    
    else if (read_pid == 0)
    {
        return cat_read(write_pid, argc, argv);
    }

    // fprintf(stderr, "Children started by parent process. \n");

    for (unsigned iter = 0; iter < Children_num; iter++)
    {
        pid_t pid = wait(NULL);
        if (pid == -1)
        {
            ERR(fprintf(stderr, "wait() failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    // fprintf(stderr, "Children terminated. Parent process terminated. \n");

    return 0;
}

//---------------------------------------------------------

static int cat_read(const pid_t write_pid, const int argc, const char** argv)
{
    // fprintf(stderr, "Child cat_read() started. \n");

    assert(argv);
    int err = 0;

    if (argc > 1)
    {
        for (unsigned iter = 1; iter < argc; iter++)
        {
            const char* filename = argv[iter];

            int fd = open_file(filename);
            if (fd < 0)
                return fd;

            err = cat_read_send_file(write_pid, fd);
            if (err != 0)
                return err;

            err = close_file(fd);
            if (err != 0)
                return err;
        }
    }
    else 
    {
        err = cat_read_send_file(write_pid, 0);
        if (err != 0)
            return err;
    }

    // fprintf(stderr, "Child cat_read() terminated. \n");
    return 0;
}

//---------------------------------------------------------

static int sig_queue(const pid_t pid, int signum, union sigval signal_value)
{
    while(sigqueue(pid, signum, signal_value) == -1)
    {
        if (errno == EAGAIN)
            usleep(100);
        else 
        {
            ERR(fprintf(stderr, "sigqueue() failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    return 0;
}

//---------------------------------------------------------

static int sigsend_snd(const pid_t pid, const char val)
{
    int rt_signum = SIGRTMIN + Real_time_signum;
    union sigval signal_value = {.sival_int = (char)val};

    return sig_queue(pid, rt_signum, signal_value);
}

//---------------------------------------------------------

static int sigsend_snd_stop(const pid_t pid)
{
    int rt_signum = SIGRTMIN + Real_time_signum;

    int sival_int = End_of_data_flag | 0;
    union sigval signal_value = {.sival_int = sival_int};

    return sig_queue(pid, rt_signum, signal_value);
}

//---------------------------------------------------------

static int cat_read_send_file(const pid_t write_pid, const int fd)
{
    ssize_t read_ret = 0;
    char val = 0;

    while(read_ret = read(fd, &val, sizeof(char)))
    {
        if (read_ret < 0)
        {
            ERR(fprintf(stderr, "read() system call failed: %s \n", strerror(errno)));
            return errno;
        }

        if (!read_ret)
            break;
    
        int err = sigsend_snd(write_pid, val);
        if (err != 0)  
            return err;
    }

    int err = sigsend_snd_stop(write_pid);
    if (err != 0)
        return err;

    return 0;
}

//---------------------------------------------------------

static void sigsend_handler(int signum, siginfo_t *info, void *ucontext)
{
    int rt_signum = SIGRTMIN + Real_time_signum;
    assert(rt_signum == signum && "sigsend_handler() set to wrong signal number");

    int sival_int = info->si_value.sival_int;

    if (sival_int & End_of_data_flag)
        exit(EXIT_SUCCESS);
    else 
    {
        char val = (char) sival_int;
        
        int err = write(1, &sival_int, sizeof(char));
        if (err == -1)
        {
            ERR(fprintf(stderr, "write() failed: %s \n", strerror(errno)));
            exit(-errno);
        }
    }

    return;
}

//---------------------------------------------------------

static int sigsend_change_handler(void)
{
    int rt_signum = SIGRTMIN + Real_time_signum;

    int sa_flags = SA_SIGINFO;

    sigset_t sa_mask;
    int err = sigemptyset(&sa_mask);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigemptyset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    struct sigaction newact = {.sa_sigaction = sigsend_handler, 
                               .sa_mask      = sa_mask,
                               .sa_flags     = sa_flags};

    err = sigaction(rt_signum, &newact, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int cat_write(void)
{
    // fprintf(stderr, "Child cat_write() started. \n");

    int err = 0;

    err = sigsend_change_handler();
    if (err != 0) return err;

    while (1);

    return 0;

    // while (1)
    // {
    //     char val = 0;
    //
    //     int received = sigsend_rcv(&val);
    //     if (received == 0)
    //         break;
    //     else if (received < 0)
    //         return received;
    //
    //     err = safe_write(1, &val, sizeof(char));
    //     if (err != 0)
    //         return err;
    // }
    //
    // err = sigsend_rcv_end();
    // if (err != 0) return err;
    //
    // fprintf(stderr, "Child cat_write() terminated. \n");
    // return 0;
}

//---------------------------------------------------------

static int open_file(const char* filename)
{
    int file_descr = open(filename, O_RDONLY);
    if (file_descr < 0)
    {
        ERR(fprintf(stderr, "open() system call failed: %s: %s \n", filename, strerror(errno)));
        return -errno;
    }

    return file_descr;
}

//---------------------------------------------------------

static int close_file(int file_descr)
{
    if (close(file_descr) < 0)
    {
        ERR(fprintf(stderr, "close() system call failed: %s \n", strerror(errno)));
        return errno;
    }

    return 0;
}

//---------------------------------------------------------

static ssize_t safe_write(int fd, const void* buf, size_t count)
{
    assert(buf);

    while (count)
    {
        ssize_t written = 0;
        size_t  offs    = 0;

        if ((written = write(fd, buf + offs, count)) < 0)
        {
            ERR(fprintf(stderr, "write() failed: %s \n", strerror(errno)));
            return -errno;
        }

        count -= written;
        offs  += written;
    }

    return 0;
}