#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//---------------------------------------------------------

#include "monitor.h"

//=========================================================

static int monitor_init(struct Monitor* monitor);

static int monitor_destroy(struct Monitor* monitor);

//=========================================================

static const struct timespec Timeout = {.tv_sec = 0, .tv_nsec = 2000000};

//=========================================================

int monitor_ctor(struct Monitor* monitor)
{
    assert(monitor);

    int err = 0;

    err = rbuffer_ctor(&(monitor->rbuffer));
    if (err != RBUFFER_OK)
        return err;

    err = monitor_init(monitor);
    if (err != MONITOR_OK)
        return err;

    return MONITOR_OK;
}

//---------------------------------------------------------

static int monitor_init(struct Monitor* monitor)
{
    assert(monitor);

    int err = 0;

    err = pthread_mutex_init(&(monitor->enter_mutex), NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_mutex_init() failed \n"));
        return err;
    }

    err = pthread_cond_init(&(monitor->empty_cond), NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_cond_init() failed \n"));
        return err;
    }

    err = pthread_cond_init(&(monitor->full_cond), NULL);
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_cond_init() failed \n"));
        return err;
    }

    return MONITOR_OK;
}

//---------------------------------------------------------

static int monitor_destroy(struct Monitor* monitor)
{
    assert(monitor);

    int err = 0;

    err = pthread_mutex_destroy(&(monitor->enter_mutex));
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_mutex_destroy() failed \n"));
        return err;
    }

    err = pthread_cond_destroy(&(monitor->empty_cond));
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_cond_destroy() failed \n"));
        return err;
    }

    err = pthread_cond_destroy(&(monitor->full_cond));
    if (err != 0)
    {
        ERR(fprintf(stderr, "pthread_cond_destroy() failed \n"));
        return err;
    }

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

    if (err = pthread_mutex_lock(&(monitor->enter_mutex)))
    {
        ERR(fprintf(stderr, "pthread_mutex_lock() failed\n"));
        return err; 
    }

    int read = rbuffer_read(&(monitor->rbuffer), size, addr);
    
    while (read <= 0)
    {
        if (read != RBUFFER_MT)
            return read;

        if (err = pthread_cond_timedwait(&(monitor->empty_cond), &(monitor->enter_mutex), &Timeout))
        {
            if (err == ETIMEDOUT)
                return MONITOR_TO;

            ERR(fprintf(stderr, "pthread_cond_wait() failed: %s \n", strerror(err)));
            return err;
        }

        read = rbuffer_read(&(monitor->rbuffer), size, addr);
    }

    if (err = pthread_cond_signal(&(monitor->full_cond)))
    {
        ERR(fprintf(stderr, "pthread_cond_signal() failed\n"));
                return err;
    }

    if (err = pthread_mutex_unlock(&(monitor->enter_mutex)))
    {
        ERR(fprintf(stderr, "pthread_mutex_unlock() failed\n"));
        return err; 
    }

    return read;
}

//---------------------------------------------------------

int monitor_write(struct Monitor* monitor, size_t size, const void* addr)
{
    assert(monitor);

    int err = 0;

    if (err = pthread_mutex_lock(&(monitor->enter_mutex)))
    {
        ERR(fprintf(stderr, "pthread_mutex_lock() failed\n"));
        return err; 
    }

    int written = rbuffer_write(&(monitor->rbuffer), size, addr);
    
    while (written <= 0)
    {
        if (written != RBUFFER_FL)
            return written;

        if (err = pthread_cond_wait(&(monitor->full_cond), &(monitor->enter_mutex)))
        {
            ERR(fprintf(stderr, "pthread_cond_wait() failed\n"));
                return err;
        }

        written = rbuffer_write(&(monitor->rbuffer), size, addr);
    }

    if (err = pthread_cond_signal(&(monitor->empty_cond)))
    {
        ERR(fprintf(stderr, "pthread_cond_signal() failed\n"));
                return err;
    }

    if (err = pthread_mutex_unlock(&(monitor->enter_mutex)))
    {
        ERR(fprintf(stderr, "pthread_mutex_unlock() failed\n"));
        return err; 
    }

    return written;
}
