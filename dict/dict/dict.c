#include <assert.h>

//---------------------------------------------------------

#include "dict.h"

//=========================================================

int dict_ctor(dict_t* dict, const word_methods_t* methods)
{
    assert(dict);
    assert(methods);

    int err = hasht_ctor(&dict->hasht);
    if (err != 0)
        return err;

    dict->methods = *methods;

    return 0;
}

//---------------------------------------------------------

int dict_dtor(dict_t* dict)
{
    assert(dict);

    int err = hasht_dtor(&dict->hasht);
    if (err != 0)
        return err;
    
    return 0;
}

//---------------------------------------------------------

int dict_fill(dict_t* dict, const char* input)
{
    assert(dict);
    assert(input);

    while (*input != '\0')
    {
        def_word_t word = { 0 };
        int err = dict->methods.ctor_read(&word, input);

        int res = hasht_add(&dict->hasht, &word);
        if (res == COUNT)
        {
            err = dict->methods.dtor(&word);
            if (err != 0) return err;
        }
        else if (res != ADD) 
            return res;
    }
    
    return 0;
}

//---------------------------------------------------------

int dict_show(const dict_t* dict, FILE* out_fd)
{
    assert(dict);

    const hasht_t* hasht = &dict->hasht;

    for (size_t iter = 0; iter < hasht->capacity; iter++)
    {
        list_t* cur_list = &hasht->data[iter];
        node_t* cur_node = cur_list->head;

        while(cur_node != NULL)
        {
            dict->methods.show(cur_node->data, out_fd);
            fprintf(out_fd, " | count: %lu \n", cur_node->count);
            cur_node = cur_node->next;
        }
    }

    return 0;
}


