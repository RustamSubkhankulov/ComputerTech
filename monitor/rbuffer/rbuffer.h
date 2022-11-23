#pragma once

//=========================================================

#include <stdlib.h>

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

// constructs empty Rbuffer_memory_size sized ring buffer
// returns negative value on error and RBUFFER_OK on success 

int rbuffer_ctor(struct Rbuffer* rbuffer);

//---------------------------------------------------------

// destructs ring buffer and frees all allocated memory
// returns negative value on error and RBUFFER_OK on success

int rbuffer_dtor(struct Rbuffer* rbuffer);

//---------------------------------------------------------

// tries to read size bytes to addr from buffer, returns 
// amount of bytes actually read or negative value on error 

int rbuffer_read(struct Rbuffer* rbuffer, size_t size, void* addr);

//---------------------------------------------------------

// writes size bytes from addr to buffer
// returns negative value on error and RBUFFER_OK on success

int rbuffer_write(struct Rbuffer* rbuffer, size_t size, void* addr);

//---------------------------------------------------------

// tries to read one byte from buffer, returns 
// amount of bytes actually read or negative value on error 

int rbuffer_read_char(struct Rbuffer* rbuffer, void* addr);

//---------------------------------------------------------

// writes one byte from addr to buffer
// returns negative value on error and RBUFFER_OK on success

int rbuffer_write_char(struct Rbuffer* rbuffer, void* addr);

//---------------------------------------------------------

// returns RBUFFER_FL if rbuffer is full 
// or RBUFFER_OK if it is not
// also returns negative value if rbuffer is invalid

int rbuffer_check_full(const struct  Rbuffer* rbuffer);


