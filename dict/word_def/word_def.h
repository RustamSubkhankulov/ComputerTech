#pragma once 

//=========================================================

#include <stdlib.h>
#include <stdio.h>

//=========================================================

struct Def_word;

typedef int Def_word_ctor_def (struct Def_word* def_word);
typedef int Def_word_ctor_copy(struct Def_word* def_word, struct Def_word* src);
typedef int Def_word_ctor_read(struct Def_word* def_word, char* input); 
typedef int Def_word_copy     (struct Def_word* def_word, struct Def_word* src);
typedef int Def_word_dtor     (struct Def_word* def_word);
typedef int Def_word_show     (struct Def_word* def_word, FILE* out_fd);
typedef int Def_word_is_empty (struct Def_word* def_word);

// TODO Def_word_read from FILE*

//---------------------------------------------------------

int def_word_ctor_def (struct Def_word* def_word);
int def_word_ctor_copy(struct Def_word* def_word, struct Def_word* src);
int def_word_copy     (struct Def_word* dst, struct Def_word* src);
int def_word_dtor     (struct Def_word* def_word);
int def_word_show     (struct Def_word* def_word, FILE* out_fd);
int def_word_is_empty (struct Def_word* def_word);

//=========================================================

typedef struct Def_word
{
    char* data;
    size_t len;

} word_t;

// TODO add name; choose methods

typedef struct Def_word_methods
{
    Def_word_ctor_def* ctor_def;
    Def_word_ctor_copy* ctor_copy;
    Def_word_ctor_read* ctor_read;
    Def_word_copy* copy;
    Def_word_dtor* dtor;
    Def_word_show* show;
    Def_word_is_empty* is_empty;

} word_methods_t;

//=========================================================

static const word_methods_t Default_word_methods = {.ctor_def  = def_word_ctor_def,
                                                    .ctor_copy = def_word_ctor_copy,
                                                    .ctor_read = NULL,
                                                    .copy      = def_word_copy,
                                                    .dtor      = def_word_dtor,
                                                    .show      = def_word_show,
                                                    .is_empty  = def_word_is_empty };
