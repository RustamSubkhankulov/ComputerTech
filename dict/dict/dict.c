#include <assert.h>

//---------------------------------------------------------

#include "dict.h"

//=========================================================

static int dict_increase(dict_t* dict) // TODO decreasing of size
{
    assert(dict);

    //smth
    return 0;
}

//=========================================================

int dict_ctor(dict_t* dict, const word_methods_t* methods)
{
    assert(dict);
    assert(methods);

    void* storage = calloc(Init_cap, sizeof(dict_entry_t));
    if (storage == NULL)
        return -1;

    dict->size = 0;
    dict->capacity = Init_cap;
    dict->data = (dict_entry_t*) storage;
    dict->methods = *methods;

    return 0;
}

//---------------------------------------------------------

int dict_dtor(dict_t* dict)
{
    assert(dict);

    dict_clean(dict);

    if (dict->capacity != 0)
    {
        free(dict->data);
    }

    dict->data = NULL;
    dict->size = 0;
    dict->capacity = 0;
    
    return 0;
}

//---------------------------------------------------------

int dict_fill(dict_t* dict, const char* input)
{
    assert(dict);
    assert(input);

    // smth
    return 0;
}

//---------------------------------------------------------

int dict_show(const dict_t* dict, FILE* out_fd)
{
    assert(dict);

    for (size_t iter = 0; iter < dict->size; iter++)
    {
        int err = dict->methods.show(&dict->data[iter].word, out_fd);
        if (err != 0)
            return err;

        fprintf(out_fd, "count: %lu", dict->data[iter].count);
    }

    return 0;
}

//---------------------------------------------------------

int dict_clean(dict_t* dict)
{
    assert(dict);

    for (size_t iter = 0; iter < dict->size; iter++)
    {
        int err = dict->methods.dtor(&dict->data[iter].word);
        if (err != 0)
            return err;
    }

    return 0;
}
