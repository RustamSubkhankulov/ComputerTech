#include <stdio.h>

//---------------------------------------------------------

#include "../inc/my_ls/my_ls.h"

//=========================================================

int main(const int argc, const char** argv)
{
    int err = my_ls(argc, argv);
    if (err != 0)
    {
        fprintf(stderr, "Terminating with error \n");
    }

    return err;
}