#include <stdlib.h>
#include <stdio.h>

//---------------------------------------------------------

#include "my_cp/my_cp.h"

//---------------------------------------------------------

int main(int argc, const char** argv)
{
    int err = my_cp(argc, argv);
    if (err != 0)
    {
        fprintf(stderr, "Terminated with error \n");
    }

    return err;
}
