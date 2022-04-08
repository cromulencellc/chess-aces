#include "platform/stdlib.h"

#include "acd2.h"
#include "acd2_store.h"
#include "challenge.h"
#include "debug.h"

struct key_permissions {
    const uint8_t key[ACCESS_TOKEN_KEY_SIZE];
    uint8_t permissions;
};

struct key_permissions KEY_PERMISSIONS [] = {
    /* MCD */
    { .key = MCD_KEY, .permissions = MCD_PERMISSIONS },
    /* RCMD */
    { .key = RCD_KEY, .permissions = RCD_PERMISSIONS },
    /* FCD */
    { .key = FSD_KEY, .permissions = FSD_PERMISSIONS }
};

#define NUM_KEY_PERMISSIONS \
    (sizeof(KEY_PERMISSIONS) / sizeof(struct key_permissions))

#define TOKEN_STORE_SIZE 64

struct token_store {
    uint8_t token[ACCESS_TOKEN_SIZE];
    uint8_t permissions;
};

struct token_store TOKEN_STORE[TOKEN_STORE_SIZE];

unsigned int next_token = 0;

void set_token(const uint8_t * token, uint8_t permissions) {
    if (next_token > TOKEN_STORE_SIZE) {
        next_token = 0;
    }

    memcpy(TOKEN_STORE[next_token].token, token, ACCESS_TOKEN_SIZE);
    TOKEN_STORE[next_token].permissions = permissions;
    next_token++;
}

uint8_t get_token(const uint8_t * token) {
    unsigned int i;
    for (i = 0; i < TOKEN_STORE_SIZE; i++) {
        if (memcmp(TOKEN_STORE[i].token, token, ACCESS_TOKEN_SIZE) == 0) {
            debug_string("Match on token:");
            debug_uint32(i);
            return TOKEN_STORE[i].permissions | ACCESS_TYPE_VALID;
        }
    }
    return 0;
}

uint8_t get_permissions_for_key(const uint8_t * key) {
    unsigned int i;

    for (i = 0; i < NUM_KEY_PERMISSIONS; i++) {
        if (memcmp(key, KEY_PERMISSIONS[i].key, ACCESS_TOKEN_KEY_SIZE) == 0) {
            debug_string("Match on key:");
            debug_uint32(i);
            return KEY_PERMISSIONS[i].permissions | ACCESS_TYPE_VALID;
        }
    }

    return 0;
}

#define ROTL(X, Y) ((X << Y) | (X >> (32 - Y)))

static uint32_t random_number_seed = 0x1d93af32;

void generate_token(uint8_t * token) {
    /* Let's read in some bus data to permute our random number seed */
    random_number_seed ^= DEVICE_READ_BUFFER(PRIV_BUS)[0];
    random_number_seed = ROTL(random_number_seed, 3);
    random_number_seed ^= DEVICE_READ_BUFFER(PRIV_BUS)[1];
    random_number_seed = ROTL(random_number_seed, 5);
    random_number_seed ^= DEVICE_READ_BUFFER(PRIV_BUS)[2];
    random_number_seed = ROTL(random_number_seed, 7);
    random_number_seed ^= DEVICE_READ_BUFFER(PRIV_BUS)[3];
    random_number_seed = ROTL(random_number_seed, 11);
    random_number_seed ^= DEVICE_READ_BUFFER(PRIV_BUS)[4];
    random_number_seed = ROTL(random_number_seed, 13);

    /* There we go. Perfectly random. */
    unsigned int i;
    for (i = 0; i < ACCESS_TOKEN_SIZE; i++) {
        token[i] = random_number_seed & 0xff;
        random_number_seed ^= ROTL(random_number_seed, 11);
    }
}