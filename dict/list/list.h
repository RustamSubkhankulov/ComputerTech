#pragma once 

//=========================================================

#include "../word_def/word_def.h"

enum Add_result
{
    ADD = 1,   // added word to the list for the first time
    COUNT = 2, // only count incremented
};

//=========================================================

struct Node;
typedef struct Node node_t;

struct Node
{
    def_word_t* data;
    size_t count;

    node_t* next;

};

//---------------------------------------------------------

typedef struct List
{
    size_t size;
    node_t* head;
    node_t* tail;

} list_t;

//=========================================================

int list_ctor(list_t* list);
int list_dtor(list_t* list);

int list_add(list_t* list, def_word_t* word);
node_t* list_search(list_t* list, const def_word_t* word);