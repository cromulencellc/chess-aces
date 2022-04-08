#ifndef acd_test_HEADER
#define acd_test_HEADER

#include "access_token.h"

/** Begin the battery of ACD tests */
void start_acd_tests();

/** Begin the next ACD test
* @return 1 if there is another test, and we started it.
*         0 if there are no further tests.
*/
int begin_next_acd_test();

void check_acd_test(uint8_t peer_id, const struct access_token_message * msg);

void acd_test_run_bus();

#endif