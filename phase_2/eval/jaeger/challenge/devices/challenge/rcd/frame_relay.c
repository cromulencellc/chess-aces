#include "debug.h"
#include "frame_relay.h"
#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"


void frame_relay_init(struct frame_relay * frame_relay) {
    memset(frame_relay, 0, sizeof(struct frame_relay));
}


int frame_relay_process_data(
    struct frame_relay * frame_relay,
    const uint8_t * data,
    unsigned int data_len
) {
    /* Append this data to the end of the cache */
    memcpy(
        &frame_relay->cached_data[frame_relay->cached_data_len],
        data,
        data_len
    );

    frame_relay->cached_data_len += data_len;

    while (1) {
        /* If we have a valid frame in incoming_frame_data, return 1. */
        if (    (frame_relay->incoming_frame_len >= 2)
             && (    PROT_DATA_SIZE(frame_relay->incoming_frame.flags)
                  == frame_relay->incoming_frame_len - FRAME_HEADER_SIZE)) {
            return 1;
        }

        /* Something has gone very wrong. */
        if (frame_relay->incoming_frame_len == sizeof(struct frame)) {
            return -1;
        }

        if (frame_relay->cached_data_len == 0) {
            return 0;
        }

        /* I am tired of implementing ring buffers. Must. Finish. The.
           Challenge. */
        /* Copy one byte from the cached data to our incoming frame */
        frame_relay->incoming_frame_data[frame_relay->incoming_frame_len] =
            frame_relay->cached_data[0];
        
        /* Shift cached data by one */
        memcpy(
            frame_relay->cached_data,
            &frame_relay->cached_data[1],
            frame_relay->cached_data_len - 1
        );

        /* Adjust offsets */
        frame_relay->cached_data_len--;
        frame_relay->incoming_frame_len++;
    }
}


const struct frame * frame_relay_consume(struct frame_relay * frame_relay) {
    /* If we don't have a valid frame, return NULL */
    if (    (frame_relay->incoming_frame_len < 2)
         || (    PROT_DATA_SIZE(frame_relay->incoming_frame.flags)
              != frame_relay->incoming_frame_len - FRAME_HEADER_SIZE)) {
        return NULL;
    }

    /* We have a valid frame. Set incoming_frame_len to 0, which essentially
       resets the buffer, and then return the frame. */
    frame_relay->incoming_frame_len = 0;
    return &frame_relay->incoming_frame;
}