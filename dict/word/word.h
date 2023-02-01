#pragma once 

//=========================================================

#include <stdlib.h>
#include <stdio.h>

//=========================================================

struct Word;

typedef int Word_ctor_def(struct Word* word);
typedef int Word_ctor_copy(struct Word* word, struct Word* src);
typedef int Word_ctor_read(struct Word* word, char* input); 
typedef int Word_copy(struct Word* word, struct Word* src);
typedef int Word_dtor(struct Word* word);
typedef int Word_show(struct Word* word, FILE* out_fd);

// TODO Word_read from FILE*

//---------------------------------------------------------

int word_ctor_def(struct Word* word);
int word_ctor_copy(struct Word* word, struct Word* src);
int word_copy(struct Word* dst, struct Word* src);
int word_dtor(struct Word* word);

//=========================================================

typedef struct Word
{
    char* data;
    size_t len;

} word_t;

// TODO add name; choose methods

typedef struct Word_methods
{
    Word_ctor_def* ctor_def;
    Word_ctor_copy* ctor_copy;
    Word_ctor_read* ctor_read;
    Word_copy* copy;
    Word_dtor* dtor;
    Word_show* show;

} word_methods_t;

//=========================================================

static word_methods_t Default_word_methods = {.ctor_def  = word_ctor_def,
                                              .ctor_copy = word_ctor_copy,
                                              }
