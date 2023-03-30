#ifndef JOS_KERN_KTIMER_H
#define JOS_KERN_KTIMER_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <stdint.h>

#include <kern/tsc.h>
#include <kern/timer.h>

enum Ktimer_res
{
    KTIMER_TYPE_UNSUP = 3, // Unsupported timer type
    KTIMER_TYPE_UNDEF = 2, // Undefined timer type
    KTIMER_ERR = 1, // ktimer API funcions call sequence violated
    KTIMER_OK  = 0
};

int ktimer_start(int64_t timeout_ms, enum TimerType type);
int ktimer_poll(void);
int ktimer_reset(int64_t timeout_ms, enum TimerType type);
void ktimer_stop(void);

#endif // JOS_KERN_KTIMER_H