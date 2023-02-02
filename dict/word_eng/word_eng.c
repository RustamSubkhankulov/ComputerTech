#include <assert.h>
#include <string.h>
#include <ctype.h>

//---------------------------------------------------------

#include "word_eng.h"

//=========================================================

int eng_word_read(def_word_t* def_word, const char* input)
{
    assert(def_word);
    assert(input);

    while (*input == '\n' || *input == '\t' || *input == ' ')
        continue;

    char* word_start = (char*) input;
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

    def_word->data = word_storage;
    def_word->len = len; 

    return 0;
}