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

// #define DEBUG

#ifdef DEBUG

    #define DBG(...)                                        \
                                                            \
    do                                                      \
    {                                                       \
    __VA_ARGS__;                                            \
    } while(0);

#else 

    #define DBG(...) 

#endif 

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

// static int sig_queue       (const pid_t pid, int signum, union sigval signal_value);

// static int sigsend_snd     (const pid_t pid, const char val);

// static int sigsend_snd_stop(const pid_t pid);

// static void sigsend_handler(int signum, siginfo_t *info, void *ucontext);

// static int sigsend_change_handler(void);

//---------------------------------------------------------

static int change_sigaction();
static int restore_sigaction();

static int change_procmask();
static int restore_procmask();

//---------------------------------------------------------

static int snd_val(pid_t pid, unsigned char val);
static int rcv_val(pid_t pid, unsigned char* val);

static int snd_stop(pid_t pid);

//---------------------------------------------------------

static int open_file(const char* filename);

static int close_file(int file_descr);

static ssize_t safe_write(int fd, const void* buf, size_t count);

//---------------------------------------------------------

static int cat(const int argc, const char** argv);

static int cat_write(const pid_t read_pid);

static int cat_read(const pid_t write_pid, const int argc, const char** argv); 

static int cat_read_send_file(const pid_t write_pid, const int fd);

//=========================================================

static const int Zero_sig     = SIGUSR1;
static const int  One_sig     = SIGUSR2;
static const int Feedback_sig = SIGWINCH;

static struct sigaction*     Zero_sig_oldact = NULL;
static struct sigaction*      One_sig_oldact = NULL;
static struct sigaction* Feedback_sig_oldact = NULL;

static sigset_t* Oldset = NULL;

//---------------------------------------------------------

// static bool Waiting_for_feedback = false;

//=========================================================

int main(const int argc, const char** argv)
{
    return cat(argc, argv);
}

//=========================================================

int cat(const int argc, const char** argv)
{
    assert(argv);

    int err = change_procmask();
    if (err != 0)
        return err;

    err = change_sigaction();
    if (err != 0)
        return err;
    
    pid_t ppid = getpid();

    pid_t write_pid = fork();
    if (write_pid == -1)
    {
        ERR(fprintf(stderr, "fork() failed: %s \n", strerror(errno)));
        return -errno;
    }
    else if (write_pid == 0)
    {
        return cat_write(ppid);
    }

    err = cat_read(write_pid, argc, argv);
    if (err != 0)
        return err;

    DBG(fprintf(stderr, "Children started by parent process. \n"));

    pid_t pid = wait(NULL);
    if (pid == -1)
    {
        ERR(fprintf(stderr, "wait() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = restore_procmask();
    if (err != 0)
        return err;

    err = restore_sigaction();
    if (err != 0)
        return err;

    DBG(fprintf(stderr, "Children terminated. Parent process terminated. \n"));

    return 0;
}

//---------------------------------------------------------

static int cat_read(const pid_t write_pid, const int argc, const char** argv)
{
    DBG(fprintf(stderr, "Child cat_read() started. \n"));

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

            err = snd_val(write_pid, '\n');
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

    DBG(fprintf(stderr, "SND: ran out od data, sending stop sig \n"));

    err = snd_stop(write_pid);
    if (err != 0)
        return err;

    DBG(fprintf(stderr, "Child cat_read() terminated. \n"));
    return 0;
}

//---------------------------------------------------------

static int change_sigaction()
{
    Zero_sig_oldact = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    assert(Zero_sig_oldact);

    One_sig_oldact = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    assert(One_sig_oldact);

    Feedback_sig_oldact = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    assert(Feedback_sig_oldact);

    sigset_t sa_mask;
    int err = sigemptyset(&sa_mask);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigemptyset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    int sa_flags = 0;

    struct sigaction newact = {.sa_handler = SIG_IGN,
                               .sa_mask    = sa_mask,
                               .sa_flags   = sa_flags};

    err = sigaction(Zero_sig, &newact, Zero_sig_oldact);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaction(One_sig, &newact, One_sig_oldact);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaction(Feedback_sig, &newact, Feedback_sig_oldact);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int restore_sigaction()
{
    assert(Zero_sig_oldact);
    assert(One_sig_oldact);
    assert(Feedback_sig_oldact);

    int err = 0;

    err = sigaction(Zero_sig, Zero_sig_oldact, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaction(One_sig, One_sig_oldact, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaction(Feedback_sig, Feedback_sig_oldact, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaction() failed: %s \n", strerror(errno)));
        return -errno;
    }

    free(Zero_sig_oldact);
    free(One_sig_oldact);
    free(Feedback_sig_oldact);

    return 0;
}

//---------------------------------------------------------

static int change_procmask()
{
    sigset_t mask;

    int err = sigemptyset(&mask);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigemptyset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, Zero_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, One_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, Feedback_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    Oldset = (sigset_t*) calloc(1, sizeof(sigset_t));
    assert(Oldset);

    err = sigprocmask(SIG_BLOCK, &mask, Oldset);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigprocmask() failed: %s \n", strerror(errno)));
        return -errno;
    }
    
    return 0;
}

//---------------------------------------------------------

static int restore_procmask()
{
    assert(Oldset);

    int err = sigprocmask(SIG_SETMASK, Oldset, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigprocmask() failed: %s \n", strerror(errno)));
        return -errno;
    }

    free(Oldset);

    return 0;
}

//---------------------------------------------------------

static int snd_stop(pid_t pid)
{
    int err = kill(pid, SIGTERM);
    if (err != 0)
    {
        ERR(fprintf(stderr, "kill() failed: %s \n", strerror(errno)));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int snd_val(pid_t pid, unsigned char val)
{
    sigset_t mask;
    int err = sigemptyset(&mask);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigemptyset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, Feedback_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    DBG(fprintf(stderr, "SND: sending val == %d \n", val));

    for (unsigned iter = 0; iter < __CHAR_BIT__; iter++)
    {
        int sig = ((val >> iter) & 1)? One_sig: Zero_sig;

        DBG(fprintf(stderr, "SND: Current bit == %d \n", ((val >> iter) & 1)));

        err = kill(pid, sig);
        if (err != 0)
        {
            ERR(fprintf(stderr, "kill() failed: %s \n", strerror(errno)));
            return -errno;
        }

        DBG(fprintf(stderr, "SND: sent sig, waiting feedback \n"));

        int received = 0;
        err = sigwait(&mask, &received);
        if (err != 0)
        {
            ERR(fprintf(stderr, "sigwait() failed: %s \n", strerror(err)));
            return -err;
        }

        DBG(fprintf(stderr, "SND: received sig, (sig == Feedback_sig) == %d \n", received == Feedback_sig));

        assert(received == Feedback_sig);
    }

    return 0;
}

//---------------------------------------------------------

static int rcv_val(pid_t pid, unsigned char* val)
{
    assert(val);
    unsigned char rcv = 0;

    sigset_t mask;
    int err = sigemptyset(&mask);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigemptyset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, One_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = sigaddset(&mask, Zero_sig);
    if (err != 0)
    {
        ERR(fprintf(stderr, "sigaddset() failed: %s \n", strerror(errno)));
        return -errno;
    }
    
    for (unsigned iter = 0; iter < __CHAR_BIT__; iter++)
    {
        int received = 0;
        err = sigwait(&mask, &received);
        if (err != 0)
        {
            ERR(fprintf(stderr, "sigwait() failed: %s \n", strerror(err)));
            return -err;
        }

        DBG(fprintf(stderr, "RCV: received is: One == %d ; Zero == %d \n", received == One_sig, received == Zero_sig));

        assert(received == One_sig || received == Zero_sig);

        if (received == One_sig)
            rcv |= (1 << iter);

        DBG(fprintf(stderr, "RCV: value now is %u \n", rcv));
        DBG(fprintf(stderr, "RCV: sending feedback signal \n"));

        err = kill(pid, Feedback_sig);
        if (err != 0)
        {
            ERR(fprintf(stderr, "kill() failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    DBG(fprintf(stderr, "RCV: received total %u \n", rcv));

    (*val) = rcv;
    return 0;
}

//---------------------------------------------------------

static int cat_read_send_file(const pid_t write_pid, const int fd)
{
    ssize_t read_ret = 0;
    unsigned char val = 0;

    while(read_ret = read(fd, &val, sizeof(unsigned char)))
    {
        if (read_ret < 0)
        {
            ERR(fprintf(stderr, "read() system call failed: %s \n", strerror(errno)));
            return errno;
        }

        if (!read_ret)
            break;
    
        DBG(fprintf(stderr, "SND: read val: %d \n", val));

        int err = snd_val(write_pid, val);
        if (err != 0)  
            return err;
    }

    return 0;
}

//---------------------------------------------------------

static int cat_write(const pid_t read_pid)
{
    DBG(fprintf(stderr, "Child cat_write() started. \n"));

    int err = 0;

    while (1)
    {
        unsigned char val = 0;
        err = rcv_val(read_pid, &val);
        if (err != 0)
            return err;

        DBG(fprintf(stderr, "RCV: received val: %d \n", val));

        ssize_t ret = safe_write(1, &val, sizeof(unsigned char));
        if (ret != 0)
            return ret;
    }

    return 0;
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