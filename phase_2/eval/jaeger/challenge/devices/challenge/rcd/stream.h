#ifndef stream_HEADER
#define stream_HEADER

#include "platform/platform.h"

#define STREAM_MAX_SIZE 32

enum STREAM_STATE {
    STREAM_STATE_READY,
    STREAM_STATE_TRANSMITTING
};

/** Reset the incoming streams for all peers */
void stream_reset_all_incoming();

/** Reset the incoming stream for one peer */
void stream_reset_peer_incoming(uint8_t peer_id);

/**
 * @return 1 if the stream for this peer is reset, 0 otherwise.
 */
int stream_is_reset_incoming(uint8_t peer_id);

/** Set the length of data we expect to receive for an incoming stream. */
void stream_set_length_incoming(uint8_t peer_id, uint32_t length);

/** Get a pointer to the buffer for a peer's incoming stream. */
uint8_t * stream_get_peer_data_incoming(uint8_t peer_id);

/** Reset the outgoing stream for one peer */
void stream_reset_peer_outgoing(uint8_t peer_id);

/** Set the data for a peer's outgoing stream. */
int stream_set_data_outgoing(
    uint8_t peer_id,
    const void * data,
    uint32_t length
);

/** Get a pointer to the outgoing data stream for a peer. */
const uint8_t * stream_get_data_outgoing(uint8_t peer_id);


#endif