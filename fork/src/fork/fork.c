#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

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