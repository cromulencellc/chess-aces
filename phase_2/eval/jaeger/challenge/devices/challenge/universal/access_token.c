#include "access_token.h"

#include "challenge.h"
#include "platform/stdlib.h"

void access_token_create_token_request(
    struct access_token_message * msg,
    uint8_t permissions,
    const uint8_t * key
) {
    msg->type = ACCESS_TOKEN_CREATE_REQUEST;
    msg->create_request.access_type = permissions;
    memcpy(msg->create_request.key, key, ACCESS_TOKEN_KEY_SIZE);
}

void access_token_create_verification_request(
    struct access_token_message * msg,
    const uint8_t * token
) {
    msg->type = ACCESS_TOKEN_VERIFICATION_REQUEST;
    memcpy(msg->verification_request.token, token, ACCESS_TOKEN_SIZE);
}