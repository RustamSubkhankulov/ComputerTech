#pragma once

//=========================================================

#include <stdlib.h>

//---------------------------------------------------------

#include "../argvec/argvec.h"

//=========================================================

struct Cmnds_entry
{
    const char* prog_name = NULL;

    struct Arg_vec arg_vec = { 0 };
};

//---------------------------------------------------------

struct Cmnds
{
    int size = 0;
    int cap  = 0;

    struct Cmnds_entry* data = NULL;

    char* buffer = NULL;
    int   buf_size = 0;
};

//=========================================================

int cmnds_ctor(struct Cmnds* cmnds);

int cmnds_dtor(struct Cmnds* cmnds);

int parse_args(struct Cmnds* cmnds, const int fdin);