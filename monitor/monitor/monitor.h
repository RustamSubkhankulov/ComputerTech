#pragma once

//=========================================================

#include <pthread.h>

//---------------------------------------------------------

#include "../rbuffer/rbuffer.h"
#include "../utility/err.h"

//=========================================================

struct Monitor
{
    struct Rbuffer rbuffer;

    pthread_mutex_t enter_mutex;

    pthread_cond_t  empty_cond;
    pthread_cond_t  full_cond;
};

//---------------------------------------------------------

enum Monitor_op_result
{
    MONITOR_OK = 0,
    MONITOR_IV = -1,
    MONITOR_PT = -2,
    MONITOR_TO = -3, // timeout
};

//=========================================================

int monitor_ctor(struct Monitor* monitor);

int monitor_dtor(struct Monitor* monitor);

int monitor_read (struct Monitor* monitor, size_t size, void* addr);

int monitor_write(struct Monitor* monitor, size_t size, const void* addr);

