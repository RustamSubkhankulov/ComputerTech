#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/ktimer.h>

static int64_t Timeout_ms = 0;
static enum TimerType Timer_type = UNDEF;
static uint64_t Prev_tsc = 0U;

bool Timer_started = false;
bool Timer_elapsed = false;

static int check_timer_type(enum TimerType type)
{
    switch (type)
    {
        case HPET0:
        case HPET1:
        case PIT:
        case PM:
        {
            return 0;
        }
        case RTC:
        {
            return -KTIMER_TYPE_UNSUP;
        }
        case UNDEF:
        default:
        {
            return -KTIMER_TYPE_UNDEF;
        }
    }

    return 0;
}

// Starts the ktimer. 
// Must be called only after ktimer_stop()
int ktimer_start(int64_t timeout_ms, enum TimerType type)
{
    if (timeout_ms < 0)
        return -E_INVAL;

    if (Timer_started != false
     || Timer_elapsed != false)
        return -KTIMER_ERR;

    int check_res = check_timer_type(type);
    if (check_res < 0) return check_res;

    Timeout_ms    = timeout_ms;
    Timer_type = type;

    Prev_tsc = read_tsc();

    Timer_started = true;
    return 0;
}

// Return value: 0 - timeout_ms is not over yet
//               1 - timeout_ms is over
//               negative value on error 
// Must be called only after timer_start() of timer_reset()
int ktimer_poll(void)
{
    if (Timer_started != true)
        return -KTIMER_ERR;

    if (Timer_elapsed == true)
        return 1;

    uint64_t cur_tsc = read_tsc();
    uint64_t ticks = cur_tsc - Prev_tsc;

    uint64_t frequency = get_cpu_frequency(Timer_type);
    
    uint64_t elapsed_time_ms = (ticks * 1000 / frequency);
    Prev_tsc = cur_tsc;

    Timeout_ms -= (int64_t) elapsed_time_ms;
    if (Timeout_ms <= 0)
        Timer_elapsed = true;

    return (int) Timer_elapsed;
}   

// Stops ktimer. Safe to call before ktimer_start() of ktimer_reset()
void ktimer_stop(void)
{
    Timer_started = false;
    Timer_elapsed = false;

    Timer_type = UNDEF;
    Timeout_ms = 0;
}

// Stops the ktimer and starts in with new 'timeout_ms' value
// Return value: 0 - previous timeout_ms is not over yet
//               1 - previous imeout is over
//               negative value on error 
int ktimer_reset(int64_t timeout_ms_ms, enum TimerType type)
{
    int res = ktimer_poll();
    if (res < 0) return res;

    ktimer_stop();

    int err = ktimer_start(timeout_ms_ms, type);
    if (err < 0) return err;

    return res;
}