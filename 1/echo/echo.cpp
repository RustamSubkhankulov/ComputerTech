#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------

#include "echo.h"

//---------------------------------------------------------

int my_echo(const int argc, const char** argv)
{
    int arg_ct     = 1;
    bool is_N_flag = 0;

    for (; arg_ct < argc && argv[arg_ct][0] == '-'; arg_ct++)
    {
        const char* cur_flag = argv[arg_ct];
        cur_flag++;

        if (!strcmp(cur_flag, N_flag))
        {
            is_N_flag = 1;
            continue;
        }

        // place for more flags
    }

    for (; arg_ct < argc; arg_ct++)
    {
        printf("%s", argv[arg_ct]);

        if (arg_ct < argc - 1)
            putchar(' ');
    }

    if (!is_N_flag)
        putchar('\n');

    return 0;
}
