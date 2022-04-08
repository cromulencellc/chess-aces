#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "callbacks.h"
#include "challenge.h"
#include "debug.h"
#include "rcd.h"


/* Callback declarations for bus handlers */
int abort_callback(const struct frame * frame);
int data_callback(const struct frame * frame);
int stream_initiate_callback(const struct frame * frame, void ** data_address);
void stream_complete_callback(const struct frame * frame);
int broadcast_callback(const struct frame * frame);
int send_callback(const struct frame * frame);

struct bus_proto_callbacks bus_callbacks = {
    .abort_callback = abort_callback,
    .data_callback = data_callback,
    .stream_initiate_callback = stream_initiate_callback,
    .stream_complete_callback = stream_complete_callback,
    .broadcast_callback = broadcast_callback,
    .send_callback = send_callback
};


int abort_callback(const struct frame * frame) {
    debug_string("[rcd] acd_test abort_callback");

    trigger_test_failure(0xf0);
    return 0;
}

int data_callback(const struct frame * frame) {
    /* We don't handle any non-streams... */
    trigger_test_failure(0xf1);
    return -1;
}

int stream_initiate_callback(const struct frame * frame, void ** data_address) {
    debug_string("[rcd] stream_initiate_callback");

    if (frame->length > STREAM_MAX_SIZE) {
        return -1;
    }
    
    stream_reset_peer_incoming(frame->address.from);
    stream_set_length_incoming(frame->address.from, frame->length);
    *data_address = stream_get_peer_data_incoming(frame->address.from);

    return 0;
}

void stream_complete_callback(const struct frame * frame) {
    debug_string("[rcd] stream_complete_callback");
    
    /* If this is an incoming stream we just finished*/
    if (stream_is_reset_incoming(frame->address.from) == 0) {
        /* Get the message that should have been placed into the stream buf for
        * this peer.
        */
        const struct access_token_message * msg =
            (const struct access_token_message *)
                stream_get_peer_data_incoming(frame->address.from);
        
        check_acd_test(frame->address.from, msg);

        stream_reset_peer_incoming(frame->address.from);
    }
}

int broadcast_callback(const struct frame * frame) {
    /* Ignore all broadcasts. */
    return 0;
}

int send_callback(const struct frame * frame) {
    if (WRITE_DATA_READY(MAINT_BUS) == 0) {
        uint8_t data_size = FRAME_HEADER_SIZE + (PROT_DATA_SIZE(frame->flags));

        memcpy((void *) DEVICE_WRITE_BUFFER(MAINT_BUS), frame, data_size);

        *DEVICE_WRITE_SIZE(MAINT_BUS) = data_size;
        WRITE_DATA_COMPLETE_SET(MAINT_BUS, 0);
        WRITE_DATA_READY_SET(MAINT_BUS, 1);
        return 1;
    }
    else {
        return 0;
    }
}
