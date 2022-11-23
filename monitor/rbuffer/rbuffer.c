#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------

#include "rbuffer.h"

//=========================================================

static int rbuffer_checker(const struct Rbuffer* rbuffer)
{
    assert(rbuffer);

    if (rbuffer->memory    == NULL 
     || rbuffer->read_ptr  == NULL
     || rbuffer->write_ptr == NULL
     || rbuffer->start_ptr != rbuffer->memory
     || rbuffer->end_ptr   != rbuffer->memory + Rbuffer_memory_size)
    {
        return RBUFFER_IV;
    }

    return RBUFFER_OK;
}

//=========================================================

int rbuffer_check_full(const struct  Rbuffer* rbuffer)
{
    assert(rbuffer);

    int err = 0;

    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
        return err;

    char* next_after_write_ptr = rbuffer->write_ptr + 1;

    if (next_after_write_ptr == rbuffer->end_ptr)
        next_after_write_ptr =  rbuffer->start_ptr;

    if (next_after_write_ptr == rbuffer->read_ptr)
        return RBUFFER_FL;
    
    return RBUFFER_OK;
}

//=========================================================

int rbuffer_ctor(struct Rbuffer* rbuffer)
{
    assert(rbuffer);

    char* memory = (char*) calloc(Rbuffer_memory_size, sizeof(char));
    if (memory == NULL)
    {
        fprintf(stderr, "calloc() call failed - failed to allocate rbuffer memory - %s \n", strerror(errno));
        return -errno;
    }

    rbuffer->memory    = memory;

    rbuffer->read_ptr  = memory;
    rbuffer->write_ptr = memory;

    rbuffer->start_ptr = memory;
    rbuffer->end_ptr   = memory + Rbuffer_memory_size;

    return RBUFFER_OK;
}

//---------------------------------------------------------

int rbuffer_dtor(struct Rbuffer* rbuffer)
{
    assert(rbuffer);

    int err = 0;

    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
    {
        if (rbuffer->memory != NULL)
            free(rbuffer->memory);
        return err;
    }

    rbuffer->memory    = NULL;

    rbuffer->read_ptr  = NULL;
    rbuffer->write_ptr = NULL;

    return RBUFFER_OK;
}

//---------------------------------------------------------

int rbuffer_read(struct Rbuffer* rbuffer, size_t size, void* addr)
{
    assert(rbuffer);
    assert(addr);

    int err = 0;

    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
        return err;

    for (size_t iter = 0; iter < size; iter++)
    {
        if ((err = rbuffer_read_char(rbuffer, (char*) addr + iter)) != 0)
            return err;
    }

    return RBUFFER_OK;
}

//---------------------------------------------------------

int rbuffer_write(struct Rbuffer* rbuffer, size_t size, void* addr)
{
    assert(rbuffer);
    assert(addr);

    int err = 0;
    
    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
        return err;

    for (size_t iter = 0; iter < size; iter++)
    {
        if ((err = rbuffer_write_char(rbuffer, (char*) addr + iter)) != 0)
            return err;
    }

    return RBUFFER_OK;
}

//---------------------------------------------------------

int rbuffer_read_char(struct Rbuffer* rbuffer, void* addr)
{
    assert(rbuffer);
    assert(addr);

    int err = 0;
    
    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
        return err;

    if (rbuffer->read_ptr == rbuffer->write_ptr)
        return RBUFFER_MT;

    *(char*) addr = *(rbuffer->read_ptr);
    rbuffer->read_ptr += 1;

    if (rbuffer->read_ptr == rbuffer->end_ptr)
        rbuffer->read_ptr =  rbuffer->start_ptr;

    return RBUFFER_OK;
}

//---------------------------------------------------------

int rbuffer_write_char(struct Rbuffer* rbuffer, void* addr)
{
    assert(rbuffer);
    assert(addr);

    int err = 0;
    
    if ((err = rbuffer_checker(rbuffer)) != RBUFFER_OK)
        return err;

    if ((err = rbuffer_check_full(rbuffer)) != RBUFFER_OK)
        return err;

    *(rbuffer->write_ptr) = *(char*) addr;
    rbuffer->write_ptr += 1;

    if (rbuffer->write_ptr == rbuffer->end_ptr)
        rbuffer->write_ptr =  rbuffer->start_ptr;

    return RBUFFER_OK;
}

