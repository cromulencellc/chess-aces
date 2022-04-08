#ifndef peer_HEADER
#define peer_HEADER

#include "challenge.h"
#include "platform/prot.h"


/**
 * Initialize the state of all FSD peers
 */
void peers_init();

/**
 * Invoked when we finish receiving streams from other clients on the networks.
 * @param peer_id Address of the peer sending us the request.
 * @param bus_proto_state The bus_proto_state the request was sent on. We need
 *                        this so we can respond on the correct bus.
 * @param request The request that was streamed over the bus.
 * @return 0 on success, -1 if there was an error.
 */
int process_request(
    uint8_t peer_id,
    struct bus_proto_state * bus_proto_state,
    struct fsd_request * request
);

/**
 * Invoked when we finish receiving streams from the ACD.
 * @param msg The response from the ACD. Should be of type
 *            ACCESS_TOKEN_VERIFICATION_RESPONSE.
 * @return 0 on success, non-zero on error.
 */
int process_acd_response(struct access_token_message * msg);

/**
 * Invoked when we finish sending a stream to a peer
 * @param peer_id ID of the peer we just finished sending a stream to
 */
int peer_stream_complete(uint8_t peer_id);

/**
 * Called on each interrupt to advance the state of FSD peers.
 */
void process_fsd_peers();

#endif