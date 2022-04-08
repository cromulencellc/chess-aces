#include "prot.h"

#include "platform.h"
#include "stdlib.h"


// void debug_string(const char * s);
// void debug_uint8(uint8_t u8);
// void debug_uint32(uint32_t u32);

int bus_peer_reset(struct bus_peer * peer) {
    peer->state = PEER_STATE_READY;
    peer->data_ptr = NULL;
    peer->stream_size_total = 0;
    peer->stream_size_transmitted = 0;
}


int bus_proto_state_initialize(
    struct bus_proto_state * bus_proto_state,
    uint8_t this_address,
    const struct bus_proto_callbacks * callbacks
) {
    memset(bus_proto_state, 0, sizeof(bus_proto_state));

    if (this_address >= PROT_BROADCAST_ADDRESS) {
        return -1;
    }

    bus_proto_state->this_address = this_address;
    bus_proto_state->callbacks = callbacks;
    bus_proto_state->num_blocking_send_frames = 0;

    unsigned int i;
    for (i = 0; i < PROT_MAX_PEERS; i++) {
        bus_peer_reset(&bus_proto_state->peers[i]);
    }

    return 0;
}

int bus_proto_process(
    struct bus_proto_state * bus_proto_state,
    const uint8_t * data,
    uint32_t data_len
) {
    struct frame frame;

    while (1) {
        /* Are we blocking on a send? */
        if (bus_proto_has_pending_send_frames(bus_proto_state)) {
            /*
                If we can flush the sending queue keep going. Otherwise we
                aren't going to process anymore messages until we can send
                what's in the queue.
            */
            if (bus_proto_flush_blocking_send_data(bus_proto_state) == 1) {
                break;
            }
        }

        /* Let's fill up the buffer first. */
        if (bus_proto_state->buf_size < BUS_PROTO_STATE_BUF_SIZE) {
            uint32_t bytes_to_copy =
                BUS_PROTO_STATE_BUF_SIZE - bus_proto_state->buf_size;
            if (bytes_to_copy > data_len) {
                bytes_to_copy = data_len;
            }

            memcpy(
                &(bus_proto_state->buf[bus_proto_state->buf_size]),
                data,
                bytes_to_copy
            );

            data = &data[bytes_to_copy];
            data_len -= bytes_to_copy;
            bus_proto_state->buf_size += bytes_to_copy;
        }

        /* Now we can just always work off the buffer. */

        /* If we have less than 2 bytes in the buffer, we need to wait until
           we have more data come in. */
        if (bus_proto_state->buf_size < 2) {
            break;
        }

        /* Do we have all of the data for this frame? */
        struct frame * frame = &bus_proto_state->frame;

        if ((frame->flags & PROT_FLAGS_DATA_SIZE_MASK) + 2
            < bus_proto_state->buf_size
        ) {
            break;
        }

        /* What peer is this? */
        struct bus_peer * peer = &bus_proto_state->peers[frame->address.from];

        switch (frame->flags & PROT_FLAGS_TYPE_MASK) {

        case PROT_FLAGS_TYPE_ABORT:
            if (frame->address.to == bus_proto_state->this_address) {
                bus_proto_state->callbacks->abort_callback(frame);
                bus_peer_reset(peer);
            }
            break;

        case PROT_FLAGS_TYPE_BROADCAST:
            bus_proto_state->callbacks->broadcast_callback(frame);
            break;

        case PROT_FLAGS_TYPE_DATA:
            if (frame->address.to == bus_proto_state->this_address) {
                bus_proto_state->callbacks->data_callback(frame);
            }
            break;

        case PROT_FLAGS_TYPE_STREAM: {
            /* This request is for us */
            if (frame->address.to != bus_proto_state->this_address) {
                break;
            }

            /* Peer is starting the stream */
            if (frame->flags & PROT_FLAGS_STREAM_INITIATE) {
                /* Invalid state for this peer. Reset it. */
                if (peer->state != PEER_STATE_READY) {
                    bus_proto_abort(bus_proto_state, frame->address.from);
                    bus_peer_reset(peer);
                    break;
                }

                /* Get the data pointer */
                uint8_t * data_pointer;
                int error = 
                    bus_proto_state->callbacks->stream_initiate_callback(
                        frame,
                        (void **) &data_pointer
                    );
                /* We can't handle this request */
                if (error) {
                    bus_proto_abort(bus_proto_state, frame->address.from);
                    bus_peer_reset(peer);
                    break;
                }

                /* Set up the peer to receive the stream */
                peer->data_ptr = data_pointer;
                peer->state = PEER_STATE_RECEIVING_STREAM;
                peer->stream_size_total = frame->length;
                peer->stream_size_transmitted = 0;

                /* Acknowledge the request */
                struct frame ack;
                ack.address.from = bus_proto_state->this_address;
                ack.address.to = frame->address.from;
                ack.flags = PROT_FLAGS_TYPE_STREAM_ACK;
                ack.flags |= PROT_FLAGS_STREAM_INITIATE;

                bus_proto_send(bus_proto_state, &ack);
                break;
            }

            /* We are already receiving data */
            /* Invalid state for this peer, reset it. */
            if (peer->state != PEER_STATE_RECEIVING_STREAM) {
                bus_proto_abort(bus_proto_state, frame->address.from);
                bus_peer_reset(peer);
                break;
            }
            
            /* We would receive too much data */
            if (peer->stream_size_transmitted + 
                (frame->flags & PROT_FLAGS_DATA_SIZE_MASK)
                > peer->stream_size_total
            ) {
                bus_proto_abort(bus_proto_state, frame->address.from);
                bus_peer_reset(peer);
                break;
            }
            
            /* Receive the data */
            memcpy(
                &peer->data_ptr[peer->stream_size_transmitted],
                frame->data,
                frame->flags & PROT_FLAGS_DATA_SIZE_MASK
            );
            peer->stream_size_transmitted +=
                frame->flags & PROT_FLAGS_DATA_SIZE_MASK;

            uint8_t flags = PROT_FLAGS_TYPE_STREAM_ACK;

            /* Is this the last packet for the stream? */
            if (frame->flags & PROT_FLAGS_STREAM_END) {
                /* Send the response before we trigger the callback. This allows
                   the callback to initiate a new stream back to the sender. */
                flags |= PROT_FLAGS_STREAM_END;
                bus_peer_reset(peer);
                struct frame ack;
                ack.address.from = bus_proto_state->this_address;
                ack.address.to = frame->address.from;
                ack.flags = flags;
                bus_proto_send(bus_proto_state, &ack);
                
                bus_proto_state->callbacks->stream_complete_callback(
                    frame
                );
                break;
            }

            struct frame ack;
            ack.address.from = bus_proto_state->this_address;
            ack.address.to = frame->address.from;
            ack.flags = flags;
            bus_proto_send(bus_proto_state, &ack);
            break;
        }
        case PROT_FLAGS_TYPE_STREAM_ACK: {
            /* Is this for us? */
            if (frame->address.to != bus_proto_state->this_address) {
                break;
            }

            /* Are we sending? */
            if (    (peer->state != PEER_STATE_AWAITING_ACK)
                 && (peer->state != PEER_STATE_SENDING_STREAM_COMPLETED)) {
                bus_proto_abort(bus_proto_state, frame->address.from);
                bus_peer_reset(peer);
                break;
            }

            /* Is this our final ack? */
            if (peer->state == PEER_STATE_SENDING_STREAM_COMPLETED) {
                bus_proto_state->callbacks->stream_complete_callback(frame);
                bus_peer_reset(peer);
                break;
            }

            /* Send the next chunk of data */
            uint32_t bytes_remaining = peer->stream_size_total;
            bytes_remaining -= peer->stream_size_transmitted;
            
            struct frame f;
            f.flags = 6;

            if (bytes_remaining <= 6) {
                f.flags = bytes_remaining;
                f.flags |= PROT_FLAGS_STREAM_END;
                memcpy(
                    &f.data,
                    &(peer->data_ptr[peer->stream_size_transmitted]),
                    bytes_remaining
                );
                peer->stream_size_transmitted += bytes_remaining;
                peer->state = PEER_STATE_SENDING_STREAM_COMPLETED;
            }
            else {
                f.flags = 6;
                memcpy(
                    &f.data,
                    &(peer->data_ptr[peer->stream_size_transmitted]),
                    6
                );
                peer->stream_size_transmitted += 6;
            }

            f.flags |= PROT_FLAGS_TYPE_STREAM;

            f.address.from = bus_proto_state->this_address;
            f.address.to = frame->address.from;

            bus_proto_send(bus_proto_state, &f);
            break;
        }

        }

        /* Advance the buffer */
        uint32_t frame_size = (frame->flags & PROT_FLAGS_DATA_SIZE_MASK) + 2;
        memmove(
            bus_proto_state->buf,
            &(bus_proto_state->buf[frame_size]),
            bus_proto_state->buf_size - frame_size
        );
        bus_proto_state->buf_size -= frame_size;
    }

    return 0;
}


int bus_proto_initiate_stream(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_address,
    const uint8_t * data,
    uint32_t data_len
) {
    if (    (peer_address == bus_proto_state->this_address)
         || (peer_address > PROT_MAX_PEERS)) {
        return -1;
    }

    if (bus_proto_state->peers[peer_address].state != PEER_STATE_READY) {
        return -1;
    }

    struct frame frame;
    frame.address.from = bus_proto_state->this_address;
    frame.address.to = peer_address;
    frame.flags = PROT_FLAGS_TYPE_STREAM | PROT_FLAGS_STREAM_INITIATE;
    /* This is the size of our data length. */
    frame.flags |= 4;

    frame.length = data_len;
    bus_proto_send(bus_proto_state, &frame);

    bus_proto_state->peers[peer_address].data_ptr = (uint8_t *) data;
    bus_proto_state->peers[peer_address].stream_size_total = data_len;
    bus_proto_state->peers[peer_address].stream_size_transmitted = 0;
    bus_proto_state->peers[peer_address].state = PEER_STATE_AWAITING_ACK;

    return 0;
}

int bus_proto_send_data(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_address,
    const uint8_t * data,
    uint32_t data_len
) {
    if (    (peer_address == bus_proto_state->this_address)
         || (peer_address > PROT_MAX_PEERS)) {
        return -1;
    }

    if (bus_proto_state->peers[peer_address].state != PEER_STATE_READY) {
        return -1;
    }

    if (data_len > FRAME_DATA_MAXLEN) {
        return -1;
    }

    struct frame frame;
    frame.address.from = bus_proto_state->this_address;
    frame.address.to = peer_address;
    frame.flags = PROT_FLAGS_TYPE_DATA;
    frame.flags |= data_len;
    memcpy(frame.data, data, data_len);

    bus_proto_send(bus_proto_state, &frame);

    return 0;
}

int bus_proto_send(
    struct bus_proto_state * bus_proto_state,
    const struct frame * frame
) {
    int result = bus_proto_state->callbacks->send_callback(frame);
    /* If we failed to send the frame */
    if (result == 0) {
        /* If we have no frames in the queue, make this the first frame */
        if (bus_proto_state->num_blocking_send_frames == 0) {
            memcpy(
                &bus_proto_state->blocking_send_frames[0],
                frame,
                sizeof(struct frame)
            );
            bus_proto_state->num_blocking_send_frames++;
        }
        /*
        * Otherwise add it to the queue
        */
        else if (bus_proto_state->num_blocking_send_frames < MAX_BLOCKING_SEND_FRAMES) {
            memcpy(
                &bus_proto_state->blocking_send_frames[
                    bus_proto_state->num_blocking_send_frames
                ],
                frame,
                sizeof(struct frame)
            );
            bus_proto_state->num_blocking_send_frames++;
        }
    }

    return result;
}

int bus_proto_flush_blocking_send_data(
    struct bus_proto_state * bus_proto_state
) {
    while (bus_proto_state->num_blocking_send_frames > 0) {
        int result = bus_proto_state->callbacks->send_callback(
            &bus_proto_state->blocking_send_frames[
                bus_proto_state->num_blocking_send_frames - 1
            ]
        );
        /* We sent the frame */
        if (result == 1) {
            bus_proto_state->num_blocking_send_frames--;
            /* If that was the last frame, we're done. */
            if (bus_proto_state->num_blocking_send_frames == 0) {
                return 0;
            }
        }
        else {
            return 1;
        }
    }

    return 0;
}

int bus_proto_abort(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_address
) {
    struct frame frame;
    frame.address.from = bus_proto_state->this_address;
    frame.address.to = peer_address;
    frame.flags = PROT_FLAGS_TYPE_ABORT;

    return bus_proto_send(bus_proto_state, &frame);
}

int bus_proto_has_pending_send_frames(
    struct bus_proto_state * bus_proto_state
) {
    if (bus_proto_state->num_blocking_send_frames > 0) {
        return 1;
    }
    else {
        return 0;
    }
}

enum peer_state bus_proto_peer_state(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_id
) {
    return bus_proto_state->peers[peer_id].state;
}