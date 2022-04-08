#include "rc.h"


int rc_init(
    struct rc * rc,
    void * data_pointer,
    rc_delete_callback delete_callback
) {
    rc->references = 1;
    rc->data_pointer = data_pointer;
    rc->delete_callback = delete_callback;
    return 0;
}

int rc_ref(struct rc * rc) {
    rc->references++;
    return 0;
}

int rc_unref(struct rc * rc) {
    rc->references--;
    if (rc->references == 0) {
        rc->delete_callback(rc->data_pointer);
    }
    return 0;
}