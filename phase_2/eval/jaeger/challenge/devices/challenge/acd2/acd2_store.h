#ifndef acd_store_HEADER
#define acd_store_HEADER

#include "platform/platform.h"


/** Set permissions for the given token in the token store
 * @param token A valid token (most likely created with generate_token)
 * @param permissions The permissions to set for this token
 */
void set_token(const uint8_t * token, uint8_t permissions);

/** Search the current token store for this token, and if found return its
 *  permissions.
 * @param token A buffer holding the token to find
 * @return The permissions. ACCESS_TYPE_VALID will be set if this is a valid
 *         token.
 */
uint8_t get_token(const uint8_t * token);

/** Get a permissions mask for a key
 * @param key A key from another device
 * @result A permissions mask. If ACCESS_TYPE_VALID will be set if the key was
 *         valid.
 */
uint8_t get_permissions_for_key(const uint8_t * key);

/** Generate a token
 * @param A pointer to the buf to place the generated token.
 */
void generate_token(uint8_t * token);

#endif