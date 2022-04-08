#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "acd2.h"
#include "acd2_stream.h"
#include "challenge.h"
#include "debug.h"

uint8_t * current_bus = NULL;
struct bus_proto_state * current_bus_proto_state;

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
    debug_string("[acd2] abort_callback");

    stream_reset_peer_incoming(frame->address.from);
    stream_reset_peer_outgoing(frame->address.from);
    return 0;
}

int data_callback(const struct frame * frame) {
    // debug_string("[acd2] data_callback");
    /* We don't handle any non-streams... */
    
    return -1;
}

int stream_initiate_callback(const struct frame * frame, void ** data_address) {
    // debug_string("[acd2] stream_initiate_callback");
    if (frame->length > STREAM_MAX_SIZE) {
        debug_string("[acd2] frame length is too large");
        debug_uint32(frame->length);
        return -1;
    }
    
    stream_reset_peer_incoming(frame->address.from);
    stream_set_length_incoming(frame->address.from, frame->length);
    *data_address = stream_get_peer_data_incoming(frame->address.from);

    return 0;
}

void stream_complete_callback(const struct frame * frame) {
    /* If this is an incoming stream we just finished*/
    if (stream_is_reset_incoming(frame->address.from) == 0) {
        /* Get the message that should have been placed into the stream buf for
        * this peer.
        */
        const struct access_token_message * msg =
            (const struct access_token_message *)
                stream_get_peer_data_incoming(frame->address.from);
        
        acd2_process_request(current_bus_proto_state, frame->address.from, msg);
        stream_reset_peer_incoming(frame->address.from);
    }
}

int broadcast_callback(const struct frame * frame) {
    /* Ignore all broadcasts. */
    return 0;
}

int send_callback(const struct frame * frame) {
    if (WRITE_DATA_READY(current_bus) == 0) {
        uint8_t data_size = FRAME_HEADER_SIZE + (PROT_DATA_SIZE(frame->flags));

        memcpy((void *) DEVICE_WRITE_BUFFER(current_bus), frame, data_size);

        *DEVICE_WRITE_SIZE(current_bus) = data_size;
        WRITE_DATA_COMPLETE_SET(current_bus, 0);
        WRITE_DATA_READY_SET(current_bus, 1);
        return 1;
    }
    else {
        return 0;
    }
}
