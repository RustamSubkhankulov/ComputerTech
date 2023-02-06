#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

//---------------------------------------------------------

#include "../dict/dict.h"
#include "../word_clang/word_clang.h"
#include "../word_eng/word_eng.h"

//=========================================================

static char* read_input(FILE* fd_in);

//=========================================================

int main(const int argc, const char** argv)
{
    dict_t dict = { 0 };

    word_methods_t methods = Eng_word_methods;
    int err = dict_ctor(&dict, &methods);
    if (err != 0) return err;

    const char* filename = argv[2];
    if (filename != NULL)
    {
        char* input_buf = read_input(stdin);
        if (input_buf == NULL) return -1;

        err = dict_fill(&dict, input_buf);
        if (err != 0) return err;

        err = dict_show(&dict, stdout);
        if (err != 0) return err;

        free(input_buf);
    }
    
    err = dict_dtor(&dict);
    if (err != 0) return err;
}

//------------------------------

static char* read_input(FILE* fd_in)
{
    assert(fd_in);

    int err = 0;

    err = fseek(fd_in, 0, SEEK_END);
    if (err == -1) return NULL;

    long size = ftell(fd_in);
    if (size == -1 || size == 0) return NULL;

    err = fseek(fd_in, 0, SEEK_SET);
    if (err == -1) return NULL;

    char* buffer = (char*) calloc(size + 1, sizeof(char));
    if (buffer == NULL) return NULL;

    size_t readn = fread(buffer, sizeof(char), size, fd_in);
    if (readn != size) 
    {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';

    return buffer;
}