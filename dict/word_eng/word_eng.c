#include <assert.h>
#include <string.h>
#include <ctype.h>

//---------------------------------------------------------

#include "word_eng.h"

//=========================================================

int eng_word_read(struct Def_word* def_word, char* input)
{
    assert(def_word);
    assert(input);

    struct Eng_word* eng_word = (struct Eng_word*) def_word;

    while (*input == '\n' || *input == '\t' || *input == ' ')
        continue;

    char* word_start = input;
    size_t len = 0;

    while (*input != '\0' 
       && (*input >= 'a' && *input <= 'z'))
    {
        input++;
        len++;
    }

    if (len == 0)
        return 0;

    char* word_storage = (char*) calloc(len + 1, sizeof(char));
    if (word_storage == NULL)
        return -1;

    memcpy(word_storage, word_start, len);
    *(word_start + len) = '\0';

    eng_word->def_word.data = word_storage;
    eng_word->def_word.len = len; 

    return 0;
}