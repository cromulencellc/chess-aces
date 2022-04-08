#ifndef rc_HEADER
#define rc_HEADER


typedef void (* rc_delete_callback)(void *);

/**
 * A struct for keeping track of reference-counted objected.
 */
struct rc {
    unsigned int references;
    void * data_pointer;
    rc_delete_callback delete_callback;
};


/**
 * Initialize the rc.
 * @param rc an allocated rc struct.
 */
int rc_init(
    struct rc * rc,
    void * data_pointer,
    rc_delete_callback delete_callback
);

int rc_ref(struct rc * rc);

int rc_unref(struct rc * rc);


#endif