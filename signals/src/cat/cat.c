#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

//---------------------------------------------------------

#include "../../inc/cat/cat.h"
#include "../../inc/sigsend/sigsend.h"
#include "../../utility/err.h"

//=========================================================

static int open_file(const char* filename);

static int close_file(int file_descr);

static ssize_t safe_write(int fd, const void* buf, size_t count);

//---------------------------------------------------------

static int cat_write(void);

static int cat_read(const pid_t write_pid, const int argc, const char** argv); 

static int cat_read_send_file(const pid_t write_pid, const int fd);

//---------------------------------------------------------

static const unsigned Children_num = 2;

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

    fprintf(stderr, "Children started by parent process. \n");

    for (unsigned iter = 0; iter < Children_num; iter++)
    {
        pid_t pid = wait(NULL);
        if (pid == -1)
        {
            ERR(fprintf(stderr, "wait() failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    fprintf(stderr, "Children terminated. Parent process terminated. \n");

    return 0;
}

//---------------------------------------------------------

static int cat_read(const pid_t write_pid, const int argc, const char** argv)
{
    fprintf(stderr, "Child cat_read() started. \n");

    assert(argv);
    int err = 0;

    err = sigsend_snd_start();
    if (err != 0) return err;

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

    err = sigsend_snd_end();
    if (err != 0) return err;

    fprintf(stderr, "Child cat_read() terminated. \n");
    return 0;
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
    
        int err = sigsend_snd(val);
        if (err != 0)  
            return err;
    }

    return 0;
}

//---------------------------------------------------------

static int cat_write(void)
{
    fprintf(stderr, "Child cat_write() started. \n");

    int err = 0;

    err = sigsend_rcv_start();
    if (err != 0) return err;

    while (1)
    {
        char val = 0;

        int received = sigsend_rcv(&val);
        if (received == 0)
            break;
        else if (received < 0)
            return received;

        err = safe_write(1, &val, sizeof(char));
        if (err != 0)
            return err;
    }

    err = sigsend_rcv_end();
    if (err != 0) return err;

    fprintf(stderr, "Child cat_write() terminated. \n");
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