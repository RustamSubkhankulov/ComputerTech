#include <stdlib.h>
#include <string.h>
#include <assert.h>

//---------------------------------------------------------

#include "list.h"

//=========================================================

int list_ctor(list_t* list)
{
    assert(list);

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return 0;
}

//---------------------------------------------------------

int list_dtor(list_t* list)
{
    assert(list);

    node_t* cur = list->head;
    node_t* next = 0;

    if (list->size != 0)
    {
        while (cur != NULL)
        {
            cur = list->head;
            next = cur->next;

            free(cur);
            cur = next;
        }
    }

    return 0;
}

//---------------------------------------------------------

int list_add(list_t* list, def_word_t* word)
{
    assert(list);
    assert(word);

    node_t* present = list_search(list, word);
    if (present != NULL)
    {
        present->count += 1;
        return 0;
    }

    node_t* new_node = (node_t*) calloc(1, sizeof(node_t));
    if (new_node == NULL)
        return -1;

    new_node->data  = word;
    new_node->next  = NULL;
    new_node->count = 1;

    if (list->size == 0)
    {
        list->head = new_node;
        list->tail = new_node;
        list->size = 1;
    }
    else 
    {
        list->tail->next = new_node;
        list->tail = new_node;
        list->size += 1;
    }

    return 0;
}

//---------------------------------------------------------

node_t* list_search(list_t* list, const def_word_t* word)
{
    assert(list);
    assert(word);

    if (list->size == 0)
        return NULL;
    else 
    {
        node_t* cur = list->head;

        while (cur != NULL)
        {
            if (strcmp(word->data, cur->data->data) == 0)
                return cur;

            cur = cur->next;
        }
    }
    
    return NULL;
}