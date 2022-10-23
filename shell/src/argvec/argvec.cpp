#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------

#include "../../inc/argvec/argvec.h"
#include "../../inc/utility/utility.h"

//=========================================================

static const int Arg_vec_init_cap    = 1;

static const int Arg_vec_resize_step = 2; 

//=========================================================

int arg_vec_ctor(struct Arg_vec* arg_vec)
{
    assert(arg_vec);

    arg_vec->cap  = Arg_vec_init_cap;
    arg_vec->size = 0;

    arg_vec->data = (char**) calloc(Arg_vec_init_cap, sizeof(char*));
    if (arg_vec->data == NULL)
    {
        fprintf(stderr, "calloc() failed to allocate mem \n");
        return -1;
    } 

    return 0;
}

//---------------------------------------------------------

int arg_vec_dtor(struct Arg_vec* arg_vec)
{
    assert(arg_vec);

    if (arg_vec->data)
        free(arg_vec->data);

    arg_vec->cap  = 0;
    arg_vec->size = 0;

    return 0;
}

//---------------------------------------------------------

int arg_vec_copy_assignment(struct Arg_vec* lv, const struct Arg_vec* rv)
{
    assert(rv);
    assert(lv);

    if (lv->data)
        free(lv->data);

    lv->size = rv->size;
    lv->cap  = rv->size;

    if (rv->data == NULL)
    {
        lv->data = NULL;
        return 0;
    }

    char** data = (char**) calloc(rv->size, sizeof(char*));
    if (data == NULL)
    {
        fprintf(stderr, "calloc() failed to allocate mem \n");
        return -1;
    }

    lv->data = data;

    memcpy(lv->data, rv->data, rv->size * sizeof(char*));

    return 0;
}

//---------------------------------------------------------

int arg_vec_increase(struct Arg_vec* arg_vec)
{
    assert(arg_vec);

    int old_cap = arg_vec->cap;
    int new_cap = old_cap * Arg_vec_resize_step;

    arg_vec->data = (char**) my_recalloc(arg_vec->data, new_cap,
                                                        old_cap,
                                                        sizeof(char*));
    if (arg_vec->data == NULL)
    {
        fprintf(stderr, "my_recalloc() failed \n");
        return -1;
    }

    arg_vec->cap = new_cap;

    return 0;
}

//---------------------------------------------------------

int arg_vec_push_back(struct Arg_vec* arg_vec, char* item)
{
    assert(arg_vec);

    if (arg_vec->size >= arg_vec->cap)
    {
        int err = arg_vec_increase(arg_vec);
        if (err < 0)
        {
            fprintf(stderr, "arg_vec_increase() failed \n");
            return err;
        }
    }

    int size = arg_vec->size;

    arg_vec->data[size] = item;
    arg_vec->size += 1;

    return 0;
}