#include <stdio.h>

//---------------------------------------------------------

#include "../inc/my_shell/my_shell.h"

//=========================================================

int main(const int argc, const char** argv)
{
    int err = my_shell(argc, argv);
    if (err != 0)
    {
        fprintf(stderr, "Terminating with error \n");
    }

    return err;
}