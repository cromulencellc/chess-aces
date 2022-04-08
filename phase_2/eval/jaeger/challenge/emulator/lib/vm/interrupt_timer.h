#ifndef interrupt_timer_HEADER
#define interrupt_timer_HEADER



int interrupt_timer_init();

/**
 * Registers a callback, returning an identifier for the callback.
 * @param callback The callback to be called each time the timer executes
 * @param data Data to be passed to the callback
 * @return <0 on error, or the identifier for this calllback
 */
int interrupt_timer_register(void (* callback) (void * data), void * data);

/**
 * Unregisters a callback.
 * @param identifier The identifier that was returned when this callback was
 *                   registered
 * @return 0 on success, non-zero on error.
 */
int interrupt_timer_unregister(int identifier);


#endif