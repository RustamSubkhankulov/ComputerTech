#pragma once

//=========================================================

#include <ctype.h>

//---------------------------------------------------------

#include "../word_def/word_def.h"

//=========================================================

int clang_word_read(def_word_t* def_word, const char* input);

//=========================================================

static const word_methods_t Clang_word_methods = {.ctor_def  = Default_word_methods.ctor_def, //
                                                  .ctor_copy = Default_word_methods.ctor_copy,
                                                  .ctor_read = clang_word_read,
                                                  .copy      = Default_word_methods.copy,
                                                  .dtor      = Default_word_methods.dtor,
                                                  .show      = Default_word_methods.show,
                                                  .is_empty  = Default_word_methods.is_empty };