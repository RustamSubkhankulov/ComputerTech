

//---------------------------------------------------------

#include "monitor.h"

//=========================================================

static int monitor_checker(const struct Monitor* monitor)
{
    assert(monitor);

    return MONITOR_OK;    
}

//=========================================================

int monitor_ctor(struct Monitor* monitor)
{
    assert(monitor);

    int err = 0;

    err = rbuffer_ctor(&(monitor->rbuffer));
    if (err != RBUFFER_OK)
        return err;

    return MONITOR_OK;
}

//---------------------------------------------------------

int monitor_dtor(struct Monitor* monitor)
{
    assert(monitor);

    int err = 0;

    err = rbuffer_dtor(&(monitor->rbuffer));
    if (err != RBUFFER_OK)
        return err;

    return MONITOR_OK;
}

//---------------------------------------------------------

int monitor_read(struct Monitor* monitor, size_t size, void* addr)
{
    assert(monitor);

    int err = 0;

    // prologue

    err = rbuffer_read(&(monitor->rbuffer), size, addr);
    if (err != RBUFFER_OK)
        return err;
    
    // epilogue

    return MONITOR_OK;
}

//---------------------------------------------------------

int monitor_write(struct Monitor* monitor, size_t size, void* addr)
{
    assert(monitor);

    int err = 0;

    // prologue

    err = rbuffer_write(&(monitor->rbuffer), size, addr);
    if (err != RBUFFER_OK)
        return err;

    // epilogue

    return MONITOR_OK;
}
