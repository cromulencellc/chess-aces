#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "stream.h"

#define STREAM_PEER_ID_NONE 0xff


struct stream_state {
    enum STREAM_STATE state;
    uint8_t peer_id;
    unsigned int length;
    uint8_t stream[STREAM_MAX_SIZE];
};

struct stream_state STREAM_STATES_INCOMING[PROT_MAX_PEERS];
struct stream_state STREAM_STATES_OUTGOING[PROT_MAX_PEERS];

void stream_reset_all_incoming() {
    unsigned int i;
    for (i = 0; i < PROT_MAX_PEERS; i++) {
        stream_reset_peer_incoming(i);
    }
}

void stream_reset_peer_incoming(uint8_t peer_id) {
    STREAM_STATES_INCOMING[peer_id].peer_id = peer_id;
    STREAM_STATES_INCOMING[peer_id].length = 0;
}

int stream_is_reset_incoming(uint8_t peer_id) {
    return STREAM_STATES_INCOMING[peer_id].length == 0 ? 1 : 0;
}

void stream_set_length_incoming(uint8_t peer_id, unsigned int length) {
    STREAM_STATES_INCOMING[peer_id].length = length;
}

uint8_t * stream_get_peer_data_incoming(uint8_t peer_id) {
    return STREAM_STATES_INCOMING[peer_id].stream;
}

void stream_reset_peer_outgoing(uint8_t peer_id) {
    STREAM_STATES_OUTGOING[peer_id].peer_id = peer_id;
    STREAM_STATES_OUTGOING[peer_id].length = 0;
}

int stream_set_data_outgoing(
    uint8_t peer_id,
    const void * data,
    uint32_t length
) {
    if (length > STREAM_MAX_SIZE) {
        return -1;
    }

    memcpy(STREAM_STATES_OUTGOING[peer_id].stream, data, length);
    STREAM_STATES_OUTGOING[peer_id].length = length;

    return 0;
}

const uint8_t * stream_get_data_outgoing(uint8_t peer_id) {
    return STREAM_STATES_OUTGOING[peer_id].stream;
}