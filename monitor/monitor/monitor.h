#pragma once

//=========================================================

#include "../rbuffer/rbuffer.h"

//=========================================================

struct Monitor
{
    struct Rbuffer rbuffer;
};

//---------------------------------------------------------

enum Monitor_op_result
{
    MONITOR_OK = 0,
    MONITOR_IV = -1,
};

//=========================================================

int monitor_ctor(struct Monitor* monitor);

int monitor_dtor(struct Monitor* monitor);

int monitor_read(struct Monitor* monitor, size_t size, void* addr);

int monitor_write(struct Monitor* monitor, size_t size, void* addr);

