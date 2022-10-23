#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

//---------------------------------------------------------

#include "../../inc/my_ls/my_ls.h"
#include "../../inc/coloredo/coloredo.h"

//=========================================================

static const int Option_l = 0;
static const int Option_a = 1;
static const int Option_i = 2;
static const int Option_n = 3;
static const int Option_R = 4;

static const int Num_opt  = 5;

static const char* Opt_str_names[] = { "long",
                                       "all",
                                       "inode",
                                       "numeric-uid-gid",
                                       "recursive"};

//---------------------------------------------------------

enum Print_info_mode
{
    SILENT     = 0x0,
    PRINT_NAME = 0x1,
};

//=========================================================

static int open_file(const char* filename, int flags);

static int close_file(const int file_descr);

static int get_options(const int argc, const char** argv, int options[]);

static int list_directory(const char* dir_name, const int* options, 
                                              Print_info_mode mode);

static int list_dir_recursive(const char* par_path, DIR* dir_stream, const int* options);

static int entry_print_info    (int dirfd, struct dirent* entry, const int* options);

static int entry_print_add_info(int dirfd, struct dirent* entry, const int* options);

static void print_access_rights(mode_t mode);

static int print_owner_name_n_group(const struct stat* file_stat);

static int print_last_mod_time(const struct stat* file_stat);

static int is_executable(int dirfd, struct dirent* entry);

//=========================================================

int my_ls(const int argc, const char** argv)
{
    int err = 0;
    int options[Num_opt] = { 0 };

    optind = get_options(argc, argv, options);
    if (optind < 0)
        return optind;

    int argnum = argc - optind;
    if (argnum == 0)
    {
        err = list_directory(".", options, SILENT);
    }
    else
    {
        Print_info_mode mode = (argnum == 1)? SILENT: PRINT_NAME;
        
        if (options[Option_R] == 1) 
            mode = PRINT_NAME;

        for (unsigned iter = optind; iter < argc; iter++)
        {
            char path[255 * 2] = { 0 };
            
            path[0] = '.';
            path[1] = '/';
            
            strcat(path, argv[iter]);

            err = list_directory(path, options, mode);
            if (err < 0)
                return err;
        }
    }

    return err;
}

//---------------------------------------------------------

static int list_directory(const char* dir_name, const int* options, 
                                              Print_info_mode mode)
{
    assert(dir_name);

    int dirfd = open_file(dir_name, O_DIRECTORY);
    if (dirfd < 0)
        return dirfd;

    if (mode == PRINT_NAME)
        printf("%s:\n", dir_name);

    DIR* dir_stream = opendir(dir_name);
    if (dir_stream == NULL)
    {
        fprintf(stderr, "opendir() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    int total_blocks = 0;

    while(1)
    {
        errno = 0;

        struct dirent* entry = readdir(dir_stream);
        if (entry == NULL && errno != 0)
        {
            fprintf(stderr, "readdir() syscall failed: %s \n", strerror(errno));
            return -errno;
        }

        if (entry == NULL && errno == 0)
            break;

        int blocks = entry_print_info(dirfd, entry, options);
        if (blocks < 0)
            return blocks;

        total_blocks += blocks;
    }

    if (options[Option_l] == 1 || options[Option_n] == 1)
        printf("\n" "total blocks (512b): %d", total_blocks);

    putchar('\n');

    if (mode == PRINT_NAME)
        putchar('\n');

    if (options[Option_R] == 1)
        return list_dir_recursive(dir_name, dir_stream, options);

    int err = closedir(dir_stream);
    if (err == -1)
    {
        fprintf(stderr, "closedir() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = close_file(dirfd);
    if (err != 0)
        return -err;

    return 0;
}

//---------------------------------------------------------

static int list_dir_recursive(const char* par_path, DIR* dir_stream, const int* options)
{
    assert(dir_stream);
    assert(options);

    rewinddir(dir_stream);

    while(1)
    {
        errno = 0;

        struct dirent* entry = readdir(dir_stream);
        if (entry == NULL && errno != 0)
        {
            fprintf(stderr, "readdir() syscall failed: %s \n", strerror(errno));
            return -errno;
        }

        if (entry == NULL && errno == 0)
            break;

        if (entry->d_type == DT_DIR)
        {
            if ((int16_t) entry->d_name[0] == '..'
            || (int8_t) entry->d_name[0] == '.')
            {
                continue;
            }

            char path[4096] = { 0 };
            
            strcpy(path, par_path);
            path[strlen(path)] = '/';

            strcat(path, entry->d_name);

            int err = list_directory(path, options, PRINT_NAME);
            if (err < 0)
                return err;
        }
    }

    return 0;
}

//---------------------------------------------------------

static int entry_print_info(int dirfd, struct dirent* entry, const int* options)
{
    assert(entry);

    if (options[Option_a] != 1)
    {
        if ((int16_t) entry->d_name[0] == '..'
        || (int8_t) entry->d_name[0] == '.')
        {
            return 0;
        }
    }

    const char* color = NULL;
    const char* style = NULL;

    switch(entry->d_type)
    {
        case DT_DIR: 
        {
            color = BLUE;  
            style = BOLD;
            break;
        }

        case DT_REG: 
        {
            int is_exec = is_executable(dirfd, entry);
            if (is_exec < 0)
                return is_exec;

            if (is_exec)
            {
                color = GREEN;
                style = BOLD;
            }
            else
            {
                color = WHITE; 
                style = NULL;
            }
            break;
        }

        default:     
        {
            color = CYAN; 
            style = BOLD;
            break;
        }
    }

    int blocks = 0;

    if (options[Option_l] == 1 || options[Option_n] == 1)
    {
        putchar('\n');

        blocks = entry_print_add_info(dirfd, entry, options);
        if (blocks < 0)
            return blocks;
    }

    print_color(color, entry->d_name, style);
    putchar(' ');

    return blocks;
}

//---------------------------------------------------------

static int is_executable(int dirfd, struct dirent* entry)
{
    assert(entry);

    struct stat file_stat = { 0 };

    int ret = fstatat(dirfd, entry->d_name, &file_stat, AT_SYMLINK_NOFOLLOW);
    if (ret != 0)
    {
        fprintf(stderr, "fstatat() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    mode_t mode = file_stat.st_mode;

    int executable = (mode & S_IXUSR) 
                  || (mode & S_IXGRP) 
                  || (mode & S_IXOTH);

    return executable && (entry->d_type == DT_REG);
}

//---------------------------------------------------------

static int entry_print_add_info(int dirfd, struct dirent* entry, const int* options)
{
    struct stat file_stat = {};

    int err = fstatat(dirfd, entry->d_name, &file_stat, AT_SYMLINK_NOFOLLOW);
    if (err != 0)
    {
        fprintf(stderr, "fstatat syscall failed: %s \n", strerror(errno));
        return errno;
    }

    if (options[Option_i] == 1)
    {
        printf("%d ", (int) file_stat.st_ino);
    }

    putchar((entry->d_type == DT_DIR)? 'd': '-');

    mode_t mode = file_stat.st_mode;
    print_access_rights(mode);

    printf("%d ", (int) file_stat.st_nlink);

    if (options[Option_n] == 1)
    {
        printf("%d %d ", (int) file_stat.st_uid, (int) file_stat.st_gid);
    }
    else
    {
        err = print_owner_name_n_group(&file_stat);
        if (err < 0)
            return err;
    }

    printf("%d ", (int) file_stat.st_size);

    err = print_last_mod_time(&file_stat);
    if (err < 0)
        return err;

    return (int) file_stat.st_blocks;
}

//---------------------------------------------------------

static int print_last_mod_time(const struct stat* file_stat)
{
    assert(file_stat);

    struct timespec mod_time = file_stat->st_mtim;

    char* time_str = ctime(&mod_time.tv_sec);
    if (time_str == NULL)
    {
        fprintf(stderr, "ctime() call failed: %s \n", strerror(errno));
        return -errno;
    }

    time_str[strlen(time_str) - 1] = ' ';
    printf("%s", time_str);

    return 0;
}

//---------------------------------------------------------

static int print_owner_name_n_group(const struct stat* file_stat)
{   
    assert(file_stat);

    uid_t uid = file_stat->st_uid;
    gid_t gid = file_stat->st_gid;

    struct passwd* passwd_entry = getpwuid(uid);
    if (!passwd_entry)
    {
        fprintf(stderr, "getpwuid() syscall failed: uid=%d: %s\n", uid, strerror(errno));
        return -errno;
    }

    printf("%s ", passwd_entry->pw_name);

    struct group* group_entry = getgrgid(gid);
    if (!group_entry)
    {
        fprintf(stderr, "getgrgid() syscall failed: gid=%d: %s\n", gid, strerror(errno));
        return -errno;
    }

    printf("%s ", group_entry->gr_name);

    return 0;
}

//---------------------------------------------------------


static void print_access_rights(mode_t mode)
{
    char ch = 0;

    char rights[10] = { 0 };

    rights[0] = (mode & S_IRUSR)? 'r': '-';
    rights[1] = (mode & S_IWUSR)? 'w': '-';
    rights[2] = (mode & S_IXUSR)? 'x': '-';

    rights[3] = (mode & S_IRGRP)? 'r': '-';
    rights[4] = (mode & S_IWGRP)? 'w': '-';
    rights[5] = (mode & S_IXGRP)? 'x': '-';

    rights[6] = (mode & S_IROTH)? 'r': '-';
    rights[7] = (mode & S_IWOTH)? 'w': '-';
    rights[8] = (mode & S_IXOTH)? 'x': '-';

    printf("%s ", rights);
    return;
}

//---------------------------------------------------------

static int get_options(const int argc, const char** argv, int options[])
{
    assert(argv);
    assert(options);

    int err = 0;
    int opt = 0;

    extern int optind;

    const struct option descr_opt[] = { {Opt_str_names[Option_l], 0, options + Option_l, 1},
                                        {Opt_str_names[Option_a], 0, options + Option_a, 1},
                                        {Opt_str_names[Option_i], 0, options + Option_i, 1},
                                        {Opt_str_names[Option_n], 0, options + Option_n, 1},
                                        {Opt_str_names[Option_R], 0, options + Option_R, 1},
                                        { 0                     , 0, 0                 , 0} };

    while ((opt = getopt_long(argc, (char* const*)argv, "lainR",
                              descr_opt, NULL)) != -1)
    {
        switch(opt)
        {
            case 'l': options[Option_l] = 1; break;
            case 'a': options[Option_a] = 1; break;
            case 'i': options[Option_i] = 1; break;
            case 'n': options[Option_n] = 1; break;
            case 'R': options[Option_R] = 1; break;
            case 0: 
            {
                break;
            }

            default:
            {
                fprintf(stderr, "\n incorrect opt %c \n", opt);
                err = -1;
                return err;
            }
        }
    }

    return optind;
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
