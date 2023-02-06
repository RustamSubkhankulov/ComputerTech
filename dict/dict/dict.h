#pragma once 

//=========================================================

#include <stdlib.h>
#include <stdio.h>

//---------------------------------------------------------

#include "../word_def/word_def.h"
#include "../hasht/hasht.h"

//=========================================================

typedef struct Dict
{
    hasht_t hasht;
    word_methods_t methods;

} dict_t;

//=========================================================

int dict_ctor(dict_t* dict, const word_methods_t* methods);
int dict_dtor(dict_t* dict);

int dict_fill(dict_t* dict, const char* input);
int dict_show(const dict_t* dict, FILE* out_fd);
