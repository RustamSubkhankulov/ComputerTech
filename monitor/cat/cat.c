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

//---------------------------------------------------------

#include "cat.h"

//=========================================================

static int cat_write(const int argc, const char** argv, struct Monitor* monitor);

static int cat_read (const int argc, const char** argv, struct Monitor* monitor);

static int open_file(const char* filename);

static int close_file(int file_descr);

static int read_to_rbuffer(int file_descr, struct Monitor* monitor);

//---------------------------------------------------------

static int exec_children(const int argc, const char** argv, struct Monitor* monitor);

//=========================================================

static const size_t Buffer_size = 4096;

//=========================================================

int cat(const int argc, const char** argv)
{
    assert(argv);

    int shm_id = shmget(IPC_PRIVATE, sizeof(struct Monitor), IPC_CREAT | IPC_EXCL | 0700);
    if (shm_id == -1)
    {
        fprintf(stderr, "shmget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }
    
    void* addr = shmat(shm_id, NULL, 0);
    if (addr == (void*)(-1))
    {
        fprintf(stderr, "shmat() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    memset(addr, 0, sizeof(struct Monitor));

    struct Monitor* monitor = (struct Monitor*) addr;
    int err = monitor_ctor(monitor);
    if (err != MONITOR_OK)
        return err;

    err = exec_children(argc, argv, monitor);
    if (err != 0)
        return err;

    err = monitor_dtor(monitor);
    if (err != MONITOR_OK)
        return err;

    err = shmdt(addr);
    if (err == -1)
    {
        fprintf(stderr, "shmdt() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = shmctl(shm_id, IPC_RMID, NULL);
    if (err == -1)
    {
        fprintf(stderr, "shmctl() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int exec_children(const int argc, const char** argv, struct Monitor* monitor)
{
    assert(argv);
    assert(monitor);

    // pthread

    // pid_t pid = 0;
    //
    // pid = fork();
    // if (pid == 0)
    // {
    //     return cat_write(argc, argv, monitor);
    // }
    // else if (pid == -1)
    // {
    //     fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
    //     return -errno;
    // }
    //
    // pid = fork();
    // if (pid == 0)
    // {
    //     return cat_read(argc, argv, monitor);
    // }
    // else if (pid == -1)
    // {
    //     fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
    //     return -errno;
    // }
    //
    // wait(NULL);
    // wait(NULL);

    return 0;
}

//---------------------------------------------------------

static int cat_write(const int argc, const char** argv, struct Monitor* monitor)
{
    assert(argv);


}

//---------------------------------------------------------

static int cat_read (const int argc, const char** argv, struct Monitor* monitor)
{
    assert(argv);
    assert(monitor);

    int err = 0;

    if (argc == 1)
    {
        err = read_to_rbuffer(0, monitor); // stdin
        if (err != 0)
            return err;
    }
    else 
    {
        for (unsigned iter = 1; iter < argc; iter++)
        {
            const char* filename = argv[iter];

            int input_fd = open_file(filename);
            if (input_fd < 0)
                continue;

            err = read_to_rbuffer(input_fd, monitor);
            if (err != 0)
                return err;

            if (input_fd > 0)
            {
                err = close_file(input_fd);
                if (err != 0)
                    return err;
            }
        }
    }

    return 0;
}

//---------------------------------------------------------

static int read_to_rbuffer(int file_descr, struct Monitor* monitor)
{
    assert(monitor);

    return 0;
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

static int close_file(int file_descr)
{
    if (close(file_descr) < 0)
    {
        fprintf(stderr, "close() system call failed: %s \n", strerror(errno));
        return errno;
    }

    return 0;
}

//=========================================================



// int cat(const int argc, const char** argv)
// {
//     int err = 0;

//     if (argc == 1)
//     {
//         err = write_file(0, 1);
//         if (err)
//             return err;
//     }

//     for (int iter = 1; iter < argc; iter++)
//     {
//         const char* filename = argv[iter];

//         int input_fd = open_file(filename);
//         if (input_fd < 0)
//         {
//             err = -input_fd;
//             continue;
//         }

//         err = write_file(input_fd, 1);
//         if (err)
//             return err;

//         err = close_file(input_fd);
//         if (err)
//             return err;
//     }

//     return err;
// }

// //---------------------------------------------------------

// static int open_file(const char* filename)
// {
//     int file_descr = open(filename, O_RDONLY);
//     if (file_descr < 0)
//     {
//         fprintf(stderr, "open() system call failed: %s: %s \n", filename, strerror(errno));
//         return -errno;
//     }

//     return file_descr;
// }

// //---------------------------------------------------------

// static int close_file(int file_descr)
// {
//     if (close(file_descr) < 0)
//     {
//         fprintf(stderr, "close() system call failed: %s \n", strerror(errno));
//         return errno;
//     }

//     return 0;
// }

// //---------------------------------------------------------

// ssize_t safe_write(int fd, const void* buf, size_t count)
// {
//     while (count)
//     {
//         ssize_t written;
//         if ((written = write(fd, buf, (size_t) count)) < 0)
//             return -1;

//         count -= written;
//     }

//     return 0;
// }

// //---------------------------------------------------------

// int write_file(int fd_in, int fd_out)
// {
//     char buff[Buffer_size];

//     size_t read_ret = 0;
//     while (read_ret = read(fd_in, buff, Buffer_size))
//     {
//         if (read_ret < 0)
//         {
//             fprintf(stderr, "read() system call failed: %s \n", strerror(errno));
//             return errno;
//         }

//         if (!read_ret)
//             break;

//         if (safe_write(fd_out, buff, (size_t) read_ret) < 0)
//         {
//             fprintf(stderr, "write() system call failed: %s \n", strerror(errno));
//             return errno;
//         }
//     }

//     return 0;
// }


