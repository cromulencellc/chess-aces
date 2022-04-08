#ifndef access_token_HEADER
#define access_token_HEADER

#include "platform/platform.h"
#include "challenge.h"

void access_token_create_token_request(
    struct access_token_message * msg,
    uint8_t permissions,
    const uint8_t * key
);

void access_token_create_verification_request(
    struct access_token_message * msg,
    const uint8_t * token
);

#endif