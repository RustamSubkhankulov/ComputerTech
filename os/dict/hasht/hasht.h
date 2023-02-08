#pragma once 

//=========================================================

#include "../list/list.h"

//=========================================================

static const size_t Initial_size = 128LU;
static const float Resize_factor = 0.6f;

//=========================================================

typedef struct Hasht
{
    list_t* data;

    size_t size;
    size_t capacity;

} hasht_t;

//=========================================================

int hasht_ctor(hasht_t* hasht);
int hasht_dtor(hasht_t* hasht);
int hasht_add(hasht_t* hasht, def_word_t* word);
