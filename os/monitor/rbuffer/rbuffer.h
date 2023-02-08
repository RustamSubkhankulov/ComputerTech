#pragma once

//=========================================================

#include <stdlib.h>

//---------------------------------------------------------

#include "../utility/err.h"

//=========================================================

// conf

const static unsigned Rbuffer_memory_size = 4096 * 16;

//=========================================================

struct Rbuffer
{
    char* memory;

    char* write_ptr;
    char* read_ptr;

    char* end_ptr;
    char* start_ptr;
};

//---------------------------------------------------------

enum Rbuffer_op_result
{
    RBUFFER_OK =  0,
    RBUFFER_FL = -1, // full
    RBUFFER_MT = -2, // empty
    RBUFFER_IV = -3, // invalid structure
};

//=========================================================

int rbuffer_ctor(struct Rbuffer* rbuffer);

int rbuffer_dtor(struct Rbuffer* rbuffer);

int rbuffer_read(struct Rbuffer* rbuffer, size_t size, void* addr);

int rbuffer_write(struct Rbuffer* rbuffer, size_t size,  const void* addr);

int rbuffer_read_char(struct Rbuffer* rbuffer, void* addr);

int rbuffer_write_char(struct Rbuffer* rbuffer, const void* addr);

int rbuffer_check_full(const struct  Rbuffer* rbuffer);


