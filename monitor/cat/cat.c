#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <pthread.h>
#include <sys/msg.h>

//---------------------------------------------------------

#include "cat.h"

//=========================================================

struct Cat_read_args
{
    int msgq_id;

    int argc;
    const char** argv;
    struct Monitor* monitor;
};

//---------------------------------------------------------

struct Cat_write_args
{
    int msgq_id;

    struct Monitor* monitor;
};

//=========================================================

enum Message_val
{
    STOP
};

struct Message
{
    long type;
    enum Message_val val;
};

//=========================================================

static const size_t  Cat_read_buffer_size = 128;
static const size_t Cat_write_buffer_size = 128;

//=========================================================

static void*  cat_write(void* monitor);
 
static void*  cat_read (void* args);

static int open_file(const char* filename);

static int close_file(int file_descr);

static int read_to_rbuffer(int file_descr, struct Monitor* monitor);

//---------------------------------------------------------

static int exec_children(const int argc, const char** argv, struct Monitor* monitor, int msgq_id);

//---------------------------------------------------------

static ssize_t safe_monitor_write(const void* buf, size_t count, struct Monitor* monitor);

static  ssize_t safe_write(int fd, const void* buf, size_t count);

//=========================================================

static const size_t Buffer_size = 4096;

//=========================================================

int cat(const int argc, const char** argv)
{
    assert(argv);

    struct Monitor monitor = { 0 };
    int err = monitor_ctor(&monitor);
    if (err != MONITOR_OK)
        return err;

    int msgq_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0700);
    if (msgq_id == -1)
    {
        fprintf(stderr, "msgget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = exec_children(argc, argv, &monitor, msgq_id);
    if (err != 0)
        return err;

    err = monitor_dtor(&monitor);
    if (err != MONITOR_OK)
        return err;

    err = msgctl(msgq_id, IPC_RMID, NULL);
    if (err != 0)
    {
        fprintf(stderr, "msgctl() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int exec_children(const int argc, const char** argv, struct Monitor* monitor, int msgq_id)
{
    assert(argv);
    assert(monitor);

    pthread_t write_thread, read_thread;

    int err = 0;

    struct Cat_read_args read_args = { .msgq_id = msgq_id, .argc = argc, .argv = argv, .monitor = monitor };
    
    // run threads

    err = pthread_create(&read_thread, NULL, cat_read, (void*) &read_args);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_create() failed: %s \n", strerror(errno)));
        return -errno;
    }

    struct Cat_write_args write_args = { .msgq_id = msgq_id, .monitor = monitor };

    err = pthread_create(&write_thread, NULL, cat_write, (void*) &write_args);
    if (err != 0)
    {   
        ERR(fprintf(stderr, "pthread_create() failed: %s \n", strerror(errno)));
        return -errno;
    }

    // wait for all of the threads to stop

    err = pthread_join(read_thread, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_join() failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = pthread_join(write_thread, NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_join() failed: %s \n", strerror(errno)));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static void* cat_write(void* arg)
{
    assert(arg);

    struct Cat_write_args* args = (struct Cat_write_args*) arg;
    // fprintf(stderr, "WRITE: started \n");

    char buffer[Cat_write_buffer_size];

    while(1)
    {
        int read = monitor_read(args->monitor, Cat_write_buffer_size, buffer);
        if (read < 0)
        {
            if (read == MONITOR_TO)
            {
                struct Message msg = { 0 };

                int size = msgrcv(args->msgq_id, &msg, sizeof(enum Message_val), 0, IPC_NOWAIT);
                if (size == -1)
                {
                    if (errno == EAGAIN || errno == ENOMSG)
                        continue;

                    ERR(fprintf(stderr, "msgrcv() syscall failed: %s \n", strerror(errno)));
                    return (void*) (unsigned long) -errno;
                }

                if (msg.val == STOP)
                    return (void*) 0;
                else 
                {
                    ERR(fprintf(stderr, "msgrcv(): unknown message value \n"));
                    return (void*) (-1);
                } 
            }

            ERR(fprintf(stderr, "monitor_read() failed \n"));
            return (void*) (unsigned long) read;
        }

        int err = safe_write(1, buffer, read);
        if (err != 0)
            return (void*) (unsigned long) err;
    }

    return (void*) 0;
}

//---------------------------------------------------------

static void* cat_read (void* arg)
{
    assert(arg);

    struct Cat_read_args* args = (struct Cat_read_args*) arg;
    // fprintf(stderr, "READ: started \n");

    int err = 0;

    if (args->argc == 1)
    {
        err = read_to_rbuffer(0, args->monitor); // stdin
        if (err != 0)
            return (void*) (unsigned long) err;
    }
    else 
    {
        for (unsigned iter = 1; iter < args->argc; iter++)
        {
            const char* filename = args->argv[iter];

            int input_fd = open_file(filename);
            if (input_fd < 0)
                continue;

            err = read_to_rbuffer(input_fd, args->monitor);
            if (err != 0)
                return (void*) (unsigned long) err;

            if (input_fd > 0)
            {
                err = close_file(input_fd);
                if (err != 0)
                    return (void*) (unsigned long) err;
            }
        }
    }

    struct Message msg = {.type = 1, .val = STOP};

    err = msgsnd(args->msgq_id, &msg, sizeof(enum Message_val), 0);
    if (err == -1)
    {
        ERR(fprintf(stderr, "msgsnd() syscall failed: %s \n", strerror(errno)));
        return (void*) (unsigned long) -errno;
    }

    return (void*) 0;
}

//---------------------------------------------------------

static int read_to_rbuffer(int file_descr, struct Monitor* monitor)
{
    assert(monitor);

    char buffer[Cat_read_buffer_size];
    ssize_t read_ret = 0;

    while (read_ret = read(file_descr, buffer, Cat_read_buffer_size))
    {
        if (read_ret < 0)
        {
            ERR(fprintf(stderr, "read() system call failed: %s \n", strerror(errno)));
            return errno;
        }

        if (!read_ret)
            break;

        int err = safe_monitor_write(buffer, read_ret, monitor);
        if (err != 0)
            return err;
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

static ssize_t safe_monitor_write(const void* buf, size_t count, struct Monitor* monitor)
{
    assert(buf);

    while (count)
    {
        ssize_t written = 0;
        size_t  offs    = 0;

        if ((written = monitor_write(monitor, count, buf + offs)) < 0)
        {
            ERR(fprintf(stderr, "write() failed: %s \n", strerror(errno)));
            return -errno;
        }

        count -= written;
        offs  += written;
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
