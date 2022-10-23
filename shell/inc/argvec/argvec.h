#pragma once

//=========================================================

#include <stdlib.h>

//=========================================================

struct Arg_vec
{
    int size = 0;
    int cap  = 0;

    char** data = NULL;
};

//=========================================================

int arg_vec_ctor(struct Arg_vec* arg_vec);

int arg_vec_dtor(struct Arg_vec* arg_vec);

int arg_vec_copy_assignment(struct Arg_vec* rv, const struct Arg_vec* lv);

int arg_vec_increase(struct Arg_vec* arg_vec);

int arg_vec_push_back(struct Arg_vec* arg_vec, char* item);


