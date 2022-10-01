#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

//=========================================================

#include "../../include/fork/fork.h"

//=========================================================

int fork_sort(const int argc, const char** argv)
{
    pid_t is_parent = 0;;

    for (unsigned iter = 1; iter < argc; iter++)
    {
        is_parent = fork();

        if (is_parent == 0)
        {
            int num = atoi(argv[iter]);

            if (num < 0)
            {
                fprintf(stderr, "fork_sort do not support negative numbers - %d\n", num);
                return -1;
            }

            __useconds_t usec = num * Usec_per_digit;
            usleep(usec);
            
            printf("%d ", num);
            break;
        }
    }

    for (unsigned iter = 0; iter < argc; iter++)
    {
        wait(NULL); // wait until all child processes end
    }

    if (is_parent)
    {
        putchar('\n');
    }

    return 0;
}

//---------------------------------------------------------

int fork_time(const int argc, const char** argv)
{
    const char* prog_name = argv[1];

    struct timeval start_time = {};
    int get_time_err = gettimeofday(&start_time, NULL);
    if (get_time_err == -1)
    {
        fprintf(stderr, "gettimeofday() syscall failed: %s\n", strerror(errno));
        return -1;
    }

    pid_t pid = fork();
    if (!pid)
    {
        int execv_err = execvp(prog_name, (char* const*)(argv + 1));
        if (execv_err == -1)
        {
            fprintf(stderr, "execv() system call failed: %s\n", strerror(errno));
            return -1;
        }
    }
    else
    {
        pid_t wait_err = waitpid(pid, NULL, 0);
        if (wait_err == -1)
        {
            fprintf(stderr, "wait() system call failed: %s\n", strerror(errno));
        }

        struct timeval end_time = {};
        get_time_err = gettimeofday(&end_time, NULL);
        if (get_time_err == -1)
        {
            fprintf(stderr, "gettimeofday() syscall failed: %s\n", strerror(errno));
            return -1;
        }

        struct timeval dif = {};
        timersub(&end_time, &start_time, &dif);

        putchar('\n');

        printf("Start moment: %s\n", ctime(&start_time.tv_sec));
        printf("End   moment: %s\n", ctime(&end_time  .tv_sec));

        printf("Elapsed time: %ld seconds "
                             "%ld milliseconds "
                             "%ld microsecond \n", dif.tv_sec, dif.tv_usec / 1000, 
                                                               dif.tv_usec % 1000);
    }

    return 0;
}