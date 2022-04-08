#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "callbacks.h"
#include "challenge.h"
#include "debug.h"
#include "fsd.h"
#include "peer.h"
#include "stream.h"



/* Callback declarations for bus handlers */
int abort_callback(const struct frame * frame);
int data_callback(const struct frame * frame);
int stream_initiate_callback(const struct frame * frame, void ** data_address);
void priv_stream_complete_callback(const struct frame * frame);
void rcd_stream_complete_callback(const struct frame * frame);
int broadcast_callback(const struct frame * frame);
int priv_send_callback(const struct frame * frame);
int rcd_send_callback(const struct frame * frame);

struct bus_proto_callbacks priv_bus_callbacks = {
    .abort_callback = abort_callback,
    .data_callback = data_callback,
    .stream_initiate_callback = stream_initiate_callback,
    .stream_complete_callback = priv_stream_complete_callback,
    .broadcast_callback = broadcast_callback,
    .send_callback = priv_send_callback
};

struct bus_proto_callbacks rcd_bus_callbacks = {
    .abort_callback = abort_callback,
    .data_callback = data_callback,
    .stream_initiate_callback = stream_initiate_callback,
    .stream_complete_callback = rcd_stream_complete_callback,
    .broadcast_callback = broadcast_callback,
    .send_callback = rcd_send_callback
};


int abort_callback(const struct frame * frame) {
    debug_string("[fsd] abort_callback");
    return 0;
}

int data_callback(const struct frame * frame) {
    /* We don't handle any non-streams... */
    return -1;
}

int stream_initiate_callback(const struct frame * frame, void ** data_address) {
    debug_string("[fsd] stream_initiate_callback");

    if (frame->length > STREAM_MAX_SIZE) {
        debug_string("[fsd] Sendinga bort because stream is too large");
        return -1;
    }
    
    debug_uint8(frame->address.from);
    debug_uint32(frame->length);
    stream_reset_peer_incoming(frame->address.from);
    stream_set_length_incoming(frame->address.from, frame->length);
    *data_address = stream_get_peer_data_incoming(frame->address.from);

    return 0;
}

void priv_stream_complete_callback(const struct frame * frame) {
    debug_string("[fsd] priv_stream_complete_callback");
    
    /* If this is an incoming stream we just finished*/
    if (stream_is_reset_incoming(frame->address.from) == 0) {
        /* If this is the privileged bus, and a message from ACD, we're going to
           treat it as a TOKEN VERIFICATION RESPONSE. */
        if (frame->address.from == ACD_ADDRESS) {
            process_acd_response(
                (struct access_token_message *)
                    stream_get_peer_data_incoming(frame->address.from)
            );
        }
        stream_reset_peer_incoming(frame->address.from);
    }
}

void rcd_stream_complete_callback(const struct frame * frame) {
    debug_string("[fsd] rcd_stream_complete_callback");
    
    /* If this is an incoming stream we just finished*/
    if (stream_is_reset_incoming(frame->address.from) == 0) {
        process_request(
            frame->address.from,
            &RCD_BUS_PROTO_STATE,
            (struct fsd_request *)
                stream_get_peer_data_incoming(frame->address.from)
        );
        stream_reset_peer_incoming(frame->address.from);
    }
}

int broadcast_callback(const struct frame * frame) {
    /* Ignore all broadcasts. */
    return 0;
}

int priv_send_callback(const struct frame * frame) {
    if (WRITE_DATA_READY(PRIV_BUS) == 0) {
        uint8_t data_size = FRAME_HEADER_SIZE + (PROT_DATA_SIZE(frame->flags));

        memcpy((void *) DEVICE_WRITE_BUFFER(PRIV_BUS), frame, data_size);

        *DEVICE_WRITE_SIZE(PRIV_BUS) = data_size;
        WRITE_DATA_COMPLETE_SET(PRIV_BUS, 0);
        WRITE_DATA_READY_SET(PRIV_BUS, 1);
        return 1;
    }
    else {
        return 0;
    }
}

int rcd_send_callback(const struct frame * frame) {
    if (WRITE_DATA_READY(RCD_BUS) == 0) {
        uint8_t data_size = FRAME_HEADER_SIZE + (PROT_DATA_SIZE(frame->flags));

        memcpy((void *) DEVICE_WRITE_BUFFER(RCD_BUS), frame, data_size);

        *DEVICE_WRITE_SIZE(RCD_BUS) = data_size;
        WRITE_DATA_COMPLETE_SET(RCD_BUS, 0);
        WRITE_DATA_READY_SET(RCD_BUS, 1);
        return 1;
    }
    else {
        return 0;
    }
}
