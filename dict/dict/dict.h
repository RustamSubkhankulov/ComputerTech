#pragma once 

//=========================================================

#include <stdlib.h>
#include <stdio.h>

//---------------------------------------------------------

#include "../word_def/word_def.h"

//=========================================================

static const float Resize_fill_factor = 0.6f;
static const size_t Init_cap = 256LU;

//=========================================================

typedef struct Dict_entry
{
    def_word_t word;
    size_t count;

} dict_entry_t;

//---------------------------------------------------------

typedef struct Dict
{
    size_t size;
    size_t capacity;

    word_methods_t methods;

    dict_entry_t* data;

} dict_t;

//=========================================================

int dict_ctor(dict_t* dict, const word_methods_t* methods);
int dict_dtor(dict_t* dict);

int dict_fill(dict_t* dict, const char* input);
int dict_clean(dict_t* dict);
int dict_show(const dict_t* dict, FILE* out_fd);
