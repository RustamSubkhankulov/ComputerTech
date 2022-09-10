
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

//---------------------------------------------------------

#include "my_cat.h"

//=========================================================

static int open_file(const char* filename);

static int close_file(int file_descr);

static int write_file(int fd_in, int fd_out);

//=========================================================

int my_cat(int argc, const char** argv)
{
    int err = 0;

    if (argc == 1)
    {
        err = write_file(0, 1);
        if (err)
            return err;
    }

    for (int iter = 1; iter < argc; iter++)
    {
        const char* filename = argv[iter];

        int input_fd = open_file(filename);
        if (input_fd < 0)
        {
            err = -input_fd;
            continue;
        }

        err = write_file(input_fd, 1);
        if (err)
            return err;

        err = close_file(input_fd);
        if (err)
            return err;
    }

    return err;
}

//---------------------------------------------------------

static int open_file(const char* filename)
{
    int file_descr = open(filename, O_RDONLY);
    if (file_descr < 0)
    {
        fprintf(stderr, "open() system call failed: %s: %s \n", filename, strerror(errno));
        return -errno;
    }

    return file_descr;
}

//---------------------------------------------------------

int close_file(int file_descr)
{
    if (close(file_descr) < 0)
    {
        fprintf(stderr, "close() system call failed: %s \n", strerror(errno));
        return errno;
    }

    return 0;
}

//---------------------------------------------------------

ssize_t safe_write(int fd, const void* buf, size_t count)
{
    while (count)
    {
        ssize_t written;
        if ((written = write(fd, buf, (size_t) count)) < 0)
            return -1;

        count -= written;
    }

    return 0;
}

//---------------------------------------------------------

int write_file(int fd_in, int fd_out)
{
    char buff[Buffer_size];

    while (ssize_t read_ret = read(fd_in, buff, Buffer_size))
    {
        if (read_ret < 0)
        {
            fprintf(stderr, "read() system call failed: %s \n", strerror(errno));
            return errno;
        }

        if (!read_ret)
            break;

        if (safe_write(fd_out, buff, (size_t) read_ret) < 0)
        {
            fprintf(stderr, "write() system call failed: %s \n", strerror(errno));
            return errno;
        }
    }

    return 0;
}
