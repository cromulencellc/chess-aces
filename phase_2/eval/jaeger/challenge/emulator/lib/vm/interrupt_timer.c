#include "interrupt_timer.h"

#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

struct interrupt_callback {
    int identifier;
    void (* callback) (void * data);
    void * data;
};

#define INTERRUPT_TIMER_MAX_CALLBACKS 64
#define INTERRUPT_TIMER_SECONDS 0
#define INTERRUPT_TIMER_MICROSECONDS 100000

struct interrupt_timer {
    struct interrupt_callback callbacks[INTERRUPT_TIMER_MAX_CALLBACKS];
    int next_identifier;
} INTERRUPT_TIMER;



void interrupt_timer_signal_handler(int signal) {
    unsigned int i;
    for (i = 0; i < INTERRUPT_TIMER_MAX_CALLBACKS; i++) {
        if (INTERRUPT_TIMER.callbacks[i].identifier != -1) {
            INTERRUPT_TIMER.callbacks[i].callback(
                INTERRUPT_TIMER.callbacks[i].data
            );
        }
    }
}


int interrupt_timer_init() {
    unsigned int i;

    for (i = 0; i < INTERRUPT_TIMER_MAX_CALLBACKS; i++) {
        INTERRUPT_TIMER.callbacks[i].identifier = -1;
        INTERRUPT_TIMER.callbacks[i].callback = NULL;
        INTERRUPT_TIMER.callbacks[i].data = NULL;
    }

    struct itimerval new_value;

    new_value.it_interval.tv_sec = INTERRUPT_TIMER_SECONDS;
    new_value.it_interval.tv_usec = INTERRUPT_TIMER_MICROSECONDS;
    new_value.it_value.tv_sec = 0;
    new_value.it_interval.tv_usec = 0;

    signal(SIGALRM, interrupt_timer_signal_handler);

    if (setitimer(ITIMER_REAL, &new_value, NULL)) {
        return -1;
    }
    else {
        return 0;
    }
}


int interrupt_time_register(void (* callback) (void * data), void * data) {
    unsigned int i;
    for (i = 0; i < INTERRUPT_TIMER_MAX_CALLBACKS; i++) {
        if (INTERRUPT_TIMER.callbacks[i].identifier == -1) {
            INTERRUPT_TIMER.callbacks[i].identifier =
                INTERRUPT_TIMER.next_identifier++;
            INTERRUPT_TIMER.callbacks[i].callback = callback;
            INTERRUPT_TIMER.callbacks[i].data = data;
            return INTERRUPT_TIMER.callbacks[i].identifier;
        }
    }
    return -1;
}

int interrupt_timer_unregister(int identifier) {
    unsigned int i;
    for (i = 0; i < INTERRUPT_TIMER_MAX_CALLBACKS; i++) {
        if (INTERRUPT_TIMER.callbacks[i].identifier == identifier) {
            INTERRUPT_TIMER.callbacks[i].identifier = -1;
            return 0;
        }
    }
    return -1;
}