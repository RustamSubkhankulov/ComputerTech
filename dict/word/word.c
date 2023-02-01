#include <assert.h>
#include <string.h>

//---------------------------------------------------------

#include "word.h"

//=========================================================

int word_ctor_def(struct Word* word)
{
    assert(word);

    word->data = NULL;
    word->len  = 0;

    return 0;
};

//---------------------------------------------------------

int word_ctor_copy(struct Word* word, struct Word* src)
{
    assert(word);
    assert(src);

    if (src->data = NULL)
    {
        word->data = NULL;
        word->len  = 0;

        return 0; 
    }

    word->data = (char*) alloc(src->len * sizeof(char));
    if (word->data == NULL)
        return -1;

    word->len = src->len;
    strncpy(word->data, src->data, word->len);

    return 0;
};

//---------------------------------------------------------

int word_copy(struct Word* dst, struct Word* src)
{
    assert(dst);
    assert(src);

    if (dst->len >= src->len)
    {
        strncpy(dst->data, src->data, src->len);
        dst->len = src->len;
    }
    else 
    {
        if (dst->data != NULL)
        {
            free(dst->data);
        }

        dst->data = (char*) alloc(src->len * sizeof(char));
        if (dst->data == NULL)
            return -1;

        strncpy(dst->data, src->data, src->len);
        dst->len = src->len;
    }

    return 0;
};

//---------------------------------------------------------

int word_dtor(struct Word* word)
{
    assert(word);

    if (word->data != NULL)
    {
        free(word->data);
        word->data = NULL;
    }

    word->len = 0;
    return 0;
};