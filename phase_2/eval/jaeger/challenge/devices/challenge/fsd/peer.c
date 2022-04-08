#include "peer.h"

#include "access_token.h"
#include "debug.h"
#include "fsd.h"
#include "platform/stdlib.h"
#include "stream.h"

/*
* This has confusing overlap with enum peer_state in "proto.h"
*/
enum fsd_peer_state {
    FSD_PEER_STATE_READY,
    FSD_PEER_STATE_WAITING_TO_VALIDATE_TOKEN,
    FSD_PEER_STATE_VALIDATING_TOKEN,
    FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE,
    FSD_PEER_STATE_SENDING_RESPONSE
};


struct peer {
    enum peer_state state;
    struct bus_proto_state * bus_proto_state;
    struct fsd_request request;
    struct access_token_message token_verification_request;
    struct fsd_response response;
};


struct peer PEERS[15];


void peer_reset(struct peer * peer) {
    memset(peer, 0, sizeof(struct peer));
    peer->state = PEER_STATE_READY;
}


void peers_init() {
    unsigned int i;
    for (i = 0; i < 15; i++) {
        peer_reset(&PEERS[i]);
    }
}


int process_request(
    uint8_t peer_id,
    struct bus_proto_state * bus_proto_state,
    struct fsd_request * request
) {
    if (peer_id >= 15) {
        return -1;
    }

    peer_reset(&PEERS[peer_id]);

    memcpy(&PEERS[peer_id].request, request, sizeof(struct fsd_request));
    PEERS[peer_id].bus_proto_state = bus_proto_state;

    struct access_token_message msg;

    access_token_create_verification_request(
        &PEERS[peer_id].token_verification_request,
        request->token
    );

    debug_string("[fsd] Received an FSD request, validating token");

    PEERS[peer_id].state = FSD_PEER_STATE_WAITING_TO_VALIDATE_TOKEN;

    return 0;
}


int process_acd_response(struct access_token_message * msg) {
    if (msg->type != ACCESS_TOKEN_VERIFICATION_RESPONSE) {
        return -1;
    }

    unsigned int peer_id;
    for (peer_id = 0; peer_id < 15; peer_id++) {
        if (PEERS[peer_id].state == FSD_PEER_STATE_VALIDATING_TOKEN) {
            if (memcmp(
                PEERS[peer_id].request.token,
                msg->verification_response.token,
                ACCESS_TOKEN_SIZE
            ) == 0) {
                break;
            }
        }
    }

    if (peer_id == 15) {
        return -1;
    }

    struct peer * peer = &PEERS[peer_id];

    if (    (msg->type != ACCESS_TOKEN_VERIFICATION_RESPONSE)
         || ((msg->verification_response.access_type & ACCESS_TYPE_VALID) != ACCESS_TYPE_VALID)
         || ((msg->verification_response.access_type & ACCESS_TYPE_FILESYSTEM) != ACCESS_TYPE_FILESYSTEM)) {

        debug_string("[fsd] Token failed verification");
        debug_uint8(msg->type);
        debug_uint8(msg->verification_response.access_type);
        debug_uint8(msg->verification_response.access_type & ACCESS_TYPE_VALID);
        debug_uint8(ACCESS_TYPE_VALID);
        debug_uint8(msg->verification_response.access_type & ACCESS_TYPE_FILESYSTEM);

        if (msg->type != ACCESS_TOKEN_VERIFICATION_RESPONSE) {
            debug_string("Invalid msg type");
        }
        if ((msg->verification_response.access_type & ACCESS_TYPE_VALID) != ACCESS_TYPE_VALID) {
            debug_string("Fail ACCESS_TYPE_VALID");
        }
        if ((msg->verification_response.access_type & ACCESS_TYPE_FILESYSTEM) != ACCESS_TYPE_FILESYSTEM) {
            debug_string("Fail ACCESS_TYPE_FILESYSTEM");
        }

        struct fsd_response response;
        switch (peer->request.type) {
        case FSD_PROT_FILE_EXISTS_REQUEST:
            peer->response.type = FSD_PROT_FILE_EXISTS_RESPONSE;
            peer->response.file_exists.success = 0;
            break;
        case FSD_PROT_FILE_SIZE_REQUEST:
            peer->response.type = FSD_PROT_FILE_SIZE_RESPONSE;
            peer->response.file_size.success = 0;
            break;
        case FSD_PROT_FILE_CONTENTS_REQUEST:
            peer->request.type = FSD_PROT_FILE_CONTENTS_RESPONSE;
            peer->response.file_contents.success = 0;
            break;
        }

        peer->state = FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE;
        
        return 0;
    }

    switch (peer->request.type) {
    case FSD_PROT_FILE_EXISTS_REQUEST: {
        debug_string("[fsd] Handling file exists request");
        memcpy(
            (void *) (FSD_DEVICE + FSD_FILENAME_OFFSET),
            peer->request.file_exists.filename,
            FSD_FILENAME_SIZE
        );

        *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET)) =
            FSD_ACTION_FILE_EXISTS;
        uint8_t result =
            *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET));
        
        peer->response.type = FSD_PROT_FILE_EXISTS_RESPONSE;
        peer->response.file_exists.success = result;
        peer->state = FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE;

        break;
    }
    case FSD_PROT_FILE_SIZE_REQUEST: {
        debug_string("[fsd] Handling file size request");
        unsigned int i;
        for (i = 0; i < 24; i++) {
            debug_uint8(((uint8_t *) &peer->request)[i]);
        }
        debug_string(peer->request.file_size.filename);
        debug_uint32(peer->request.type);
        memcpy(
            (void *) (FSD_DEVICE + FSD_FILENAME_OFFSET),
            peer->request.file_size.filename,
            FSD_FILENAME_SIZE
        );

        *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET)) =
            FSD_ACTION_FILE_SIZE;
        uint8_t result =
            *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET));

        peer->response.type = FSD_PROT_FILE_SIZE_RESPONSE;
        peer->response.file_size.success = result;
        peer->response.file_size.size =
            *((volatile uint32_t *) (FSD_DEVICE + FSD_SIZE_OFFSET));
        peer->state = FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE;

        break;
    }
    case FSD_PROT_FILE_CONTENTS_REQUEST: {
        debug_string("[fsd] Handling file contents request");
        memcpy(
            (void *) (FSD_DEVICE + FSD_FILENAME_OFFSET),
            peer->request.file_contents.filename,
            FSD_FILENAME_SIZE
        );
        *((volatile uint32_t *) (FSD_DEVICE + FSD_SIZE_OFFSET)) =
            peer->request.file_contents.size;
        *((volatile uint32_t *) (FSD_DEVICE + FSD_OFFSET_OFFSET)) =
            peer->request.file_contents.offset;
        
        *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET)) =
            FSD_ACTION_READ_FILE;
        
        peer->response.type = FSD_PROT_FILE_CONTENTS_RESPONSE;
        peer->response.file_contents.success =
            *((volatile uint8_t *) (FSD_DEVICE + FSD_ACTION_OFFSET));
        peer->response.file_contents.size =
            *((volatile uint32_t *) (FSD_DEVICE + FSD_SIZE_OFFSET));
        peer->response.file_contents.offset =
            *((volatile uint32_t *) (FSD_DEVICE + FSD_OFFSET_OFFSET));
        memcpy(
            peer->response.file_contents.contents,
            ((uint8_t *) (FSD_DEVICE + FSD_CONTENTS_OFFSET)),
            peer->response.file_contents.size
        );

        peer->state = FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE;

        break;
    }
    }

    return 0;
}


int peer_stream_complete(uint8_t peer_id) {
    if (PEERS[peer_id].state == FSD_PEER_STATE_SENDING_RESPONSE) {
        peer_reset(&PEERS[peer_id]);
    }
    return 0;
}


void process_fsd_peers() {
    unsigned int peer_id;
    for (peer_id = 0; peer_id < 15; peer_id++) {
        struct peer * peer = &PEERS[peer_id];

        if (peer->state == FSD_PEER_STATE_WAITING_TO_SEND_RESPONSE) {
            if (bus_proto_peer_state(peer->bus_proto_state, peer_id) == PEER_STATE_READY) {
                int err = bus_proto_initiate_stream(
                    peer->bus_proto_state,
                    peer_id,
                    (const uint8_t *) &peer->response,
                    sizeof(struct fsd_response)
                );
                if (err == 0) {
                    peer->state = FSD_PEER_STATE_SENDING_RESPONSE;
                }
            }
        }
        else if (peer->state == FSD_PEER_STATE_WAITING_TO_VALIDATE_TOKEN) {
            debug_string("We are waiting to validate token");
            if (bus_proto_peer_state(&PRIV_BUS_PROTO_STATE, ACD_ADDRESS) == PEER_STATE_READY) {
                int err = bus_proto_initiate_stream(
                    &PRIV_BUS_PROTO_STATE,
                    ACD_ADDRESS,
                    (const uint8_t *) &peer->token_verification_request,
                    sizeof(struct access_token_message)
                );
                if (err == 0) {
                    peer->state = FSD_PEER_STATE_VALIDATING_TOKEN;
                }
            }
        }
    }
}