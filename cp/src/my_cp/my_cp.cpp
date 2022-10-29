#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>

//---------------------------------------------------------

#include "my_cp.h"

//=========================================================

static const int Option_i = 0;
static const int Option_f = 1;
static const int Option_v = 2;

static const int Num_opt  = 3;

static const char* Opt_str_names[] = { "interactive",
                                       "force",
                                       "verbose"};

//=========================================================

static int open_file(const char* filename, int flags);

static int close_file(const int file_descr);

static int write_file(const int fd_in, const int fd_out);

static int get_options(const int argc, const char** argv, int options[]);

static int cp_file_to_file(const int options[], const char* src, const char* dst);

static int cp_to_dir(const int options[], int optnum, const int argc, const char* argv[]);

static int copy_access_rights(const int src_fd, const int ds_fd);

static int fchange_mod(const int fd, const mode_t mode);

static int is_dir(const char* pathname);

static int copy_file(const int src_fd, const int dst_fd);

static int ask_user(const char* pathname);

static int open_destination_dir(const int options[], const char* pathname, const int dir_fd);

//=========================================================

int my_cp(int argc, const char** argv)
{
    int err = 0; 

    int options[Num_opt] = { 0 }; // all options are enables by default

    optind = get_options(argc, argv, options);
    if (optind < 0)
        return optind;
        
    int argnum = argc - optind;

    if (argnum == 2)
    {
        if (is_dir(argv[argc - 1]))
            err = cp_to_dir(options, optind, argc, argv);
        else
            err = cp_file_to_file(options, argv[optind], argv[optind + 1]);
    }
    else if (argnum > 2)
    {
        err = cp_to_dir(options, optind, argc, argv);
    }
    else
    {
        fprintf(stderr, "cp: missing args \n");
        err = 1;
    }

    return err;
}

//---------------------------------------------------------

static int ask_user(const char* pathname)
{
    assert(pathname);
    printf("my_cp:  переписать '%s'? ", pathname);

    char symb = 0;

    int scanned  = scanf("%c", &symb);
    if (scanned != 1)
        return 0;

    while (getchar() != '\n');

    if (symb == 'y')
        return 1;

    return 0;
}

//---------------------------------------------------------

static int open_destination_dir(const int options[], const char* pathname, const int dir_fd)
{
    int open_flags = O_WRONLY | O_CREAT | 0700;
    if (options[Option_i] == 1)
        open_flags |= O_EXCL;

    int res_fd = 0;
    while ((res_fd = openat(dir_fd, pathname, open_flags)) < 0)
    {
        if (errno == EEXIST)
        {
            if (ask_user(pathname))
            {
                open_flags &= (~O_EXCL);
                continue;
            }
            else
                return 0;
        }

        if (options[Option_f] == 1)
        {
            int unlink_err = unlinkat(dir_fd, pathname, 0);
            if (unlink_err)
            {
                fprintf(stderr, "unlink() system call failed: %s: %s\n", pathname, 
                                                                    strerror(errno));
                return unlink_err;
            }
            continue;
        }

        fprintf(stderr, "openat() system call failed: %s: %s\n", pathname, strerror(errno));
        return -res_fd;
    }

    return res_fd;
}

//---------------------------------------------------------

static int cp_file_to_file(const int options[], const char* src, const char* dst)
{
    assert(options);
    assert(src);
    assert(dst);

    int dst_fd = open_destination_dir(options, dst, AT_FDCWD);
    if (dst_fd == 0)
        return 0;
    else if (dst_fd < 0)
        return -dst_fd;

    int src_fd = open_file(src, O_RDONLY);
    if (src_fd < 0)
        return -src_fd;

    int copy_err = copy_file(src_fd, dst_fd);
    if (copy_err)
        return copy_err;

    int close_err = close_file(src_fd);
    if (close_err)
        return close_err;

    close_err = close_file(dst_fd);
    if (close_err)
        return close_err;

    if (options[Option_v] == 1)
    {
        printf("'%s' -> '%s'\n", src, dst);
    }

    return 0;
}

//---------------------------------------------------------

static int copy_file(const int src_fd, const int dst_fd)
{
    int write_err = write_file(src_fd, dst_fd);
    if (write_err)
        return write_err;

    int chng_rights = copy_access_rights(src_fd, dst_fd);
    if (chng_rights)
        return chng_rights;

    return 0;
}

//---------------------------------------------------------

static int copy_access_rights(const int src_fd, const int dst_fd)
{
    struct stat src_stat = {};

    int fstat_ret = fstat(src_fd, &src_stat);
    if (fstat_ret != 0)
    {
        fprintf(stderr, "fstat() system call failed: %d: %s \n", src_fd, strerror(errno));
        return -errno;
    }

    int changed_err = fchange_mod(dst_fd, src_stat.st_mode);
    if (changed_err)
        return changed_err;

    return 0;
}

//---------------------------------------------------------

static int cp_to_dir(const int options[], int optnum, const int argc, const char* argv[])
{
    assert(options);
    assert(argv);

    const char* dirname = argv[argc - 1];
    int dir_fd = open_file(dirname, O_DIRECTORY);
    if (dir_fd < 0)
        return dir_fd;

    for (int cur_src = optnum; cur_src < argc - 1; cur_src++)
    {
        int cur_dst_fd = open_destination_dir(options, argv[cur_src], dir_fd);
        if (cur_dst_fd == 0)
            continue;
        else if (cur_dst_fd < 0)
            return cur_dst_fd;

        int cur_src_fd = open(argv[cur_src], O_RDONLY);
        if (cur_src_fd < 0)
        {
            fprintf(stderr, "open() system call failed: %s: %s \n", argv[cur_src], strerror(errno));
            return -errno;
        }

        int copy_err = copy_file(cur_src_fd, cur_dst_fd);
        if (copy_err)
            return copy_err;

        int close_err = close_file(cur_src_fd);
        if (close_err)
            return close_err;
        
        close_err = close_file(cur_dst_fd);
        if (close_err)
            return close_err;

        if (options[Option_v])
        {
            printf("'%s' -> '%s/%s'\n", argv[cur_src], dirname, 
                                        argv[cur_src]);
        }
    }

    int close_dir = close_file(dir_fd);
    if (close_dir)
        return close_dir;

    return 0;
}

//---------------------------------------------------------

static int is_dir(const char* pathname)
{
    struct stat path_stat = {};

    int stat_ret  = stat(pathname, &path_stat);
    if (stat_ret != 0)
    {
        if (errno != ENOENT)
        {
            fprintf(stderr, "stat() system call failed: %s: %s %d\n", pathname, strerror(errno), errno);
            return -errno;
        }
        else
            return 0;
    }

    return S_ISDIR(path_stat.st_mode);
}

//---------------------------------------------------------

static int get_options(const int argc, const char** argv, int* options)
{
    assert(argv);
    assert(options);

    int err = 0;
    int opt = 0;

    extern int optind;

    const struct option descr_opt[] = { {Opt_str_names[Option_i], 0, options + Option_i, 1},
                                        {Opt_str_names[Option_f], 0, options + Option_f, 1},
                                        {Opt_str_names[Option_v], 0, options + Option_v, 1},
                                        { 0                     , 0, 0                 , 0} };

    while ((opt = getopt_long(argc, (char* const*)argv, "ifv",
                              descr_opt, NULL)) != -1)
    {
        switch(opt)
        {
            case 'i': options[Option_i] = 1; break;
            case 'f': options[Option_f] = 1; break;
            case 'v': options[Option_v] = 1; break;
            case 0: 
            {
                break;
            }

            default:
            {
                fprintf(stderr, "\n returned opt %c \n", opt);
                err = -1;
                return err;
            }
        }
    }

    return optind;
}

//---------------------------------------------------------

static int fchange_mod(const int fd, const mode_t mode)
{
    int fchmod_ret = fchmod(fd, mode);
    if (fchmod_ret == -1)
    {
        fprintf(stderr, "fchmod() system call failed: %d: %s \n", fd, strerror(errno));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int open_file(const char* filename, const int flags)
{
    assert(filename);

    int file_descr = open(filename, flags);
    if (file_descr < 0)
    {
        fprintf(stderr, "open() system call failed: %s: %s \n", filename, strerror(errno));
        return -errno;
    }

    return file_descr;
}

//---------------------------------------------------------

static int close_file(const int file_descr)
{
    if (close(file_descr) < 0)
    {
        fprintf(stderr, "close() system call failed: %s \n", strerror(errno));
        return errno;
    }

    return 0;
}

//---------------------------------------------------------

ssize_t safe_write(const int fd, const void* buf, size_t count)
{
    assert(buf);

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

static int write_file(const int fd_in, const int fd_out)
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
