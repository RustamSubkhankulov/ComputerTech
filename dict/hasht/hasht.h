#pragma once 

//=========================================================

#include "../list/list.h"

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
int hasht_show(const hasht_t* hasht);
