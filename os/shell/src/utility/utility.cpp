#include <stdlib.h>
#include <assert.h>
#include <string.h>

//---------------------------------------------------------

#include "../../inc/utility/utility.h"

//=========================================================

void* my_recalloc(void* ptr, size_t number, size_t prev_number, size_t size_of_elem) 
{
    assert(ptr);

    size_t new_size = number * size_of_elem;
    void* new_ptr = realloc(ptr, new_size);

    if (new_ptr == NULL) 
        return NULL;

    if (number > prev_number) 
    {
        size_t offset = number - prev_number;
        char* base = (char*)new_ptr + size_of_elem * prev_number;

        memset(base, 0, offset * size_of_elem);
    }

    return new_ptr;
}