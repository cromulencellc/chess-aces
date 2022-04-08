#include "platform/platform.h"
#include "platform/prot.h"

#include "acd_test.h"
#include "acd_test_callbacks.h"
#include "access_token.h"
#include "challenge.h"
#include "debug.h"
#include "maint.h"
#include "stream.h"

struct bus_proto_state ACD_PROTO_STATE;

struct acd_test {
    const uint8_t key[ACCESS_TOKEN_KEY_SIZE];
    uint8_t permissions;
};

struct acd_test ACD_TESTS[] = {
    /* MCD */  { .key = MCD_KEY, .permissions = MCD_PERMISSIONS },
    /* RCD */ { .key = RCD_KEY, .permissions = RCD_PERMISSIONS },
    /* FCD */  { .key = FSD_KEY, .permissions = FSD_PERMISSIONS }
};

#define NUM_TESTS (sizeof(ACD_TESTS) / sizeof(struct acd_test))

unsigned int TEST_NUMBER = 0;

void start_acd_tests() {
    debug_string("[maint] start_acd_tests");
    bus_proto_state_initialize(&ACD_PROTO_STATE, MCD_ADDRESS, &acd_test_callbacks);

    TEST_NUMBER = 0;

    begin_next_acd_test();
}

int begin_next_acd_test() {
    debug_string("[maint] begin_next_acd_test");
    if (TEST_NUMBER >= NUM_TESTS) {
        debug_string("[maint] All tests are complete");
        return 0;
    };

    struct access_token_message msg;

    debug_string("[maint] Test number:");
    debug_uint32(TEST_NUMBER);

    access_token_create_token_request(
        &msg,
        ACD_TESTS[TEST_NUMBER].permissions,
        ACD_TESTS[TEST_NUMBER].key
    );

    stream_set_data_outgoing(
        ACD_ADDRESS,
        &msg,
        sizeof(struct access_token_message)
    );

    /* TODO: What do we do here if this call fails? */
    bus_proto_initiate_stream(
        &ACD_PROTO_STATE,
        ACD_ADDRESS,
        stream_get_data_outgoing(ACD_ADDRESS),
        sizeof(struct access_token_message)
    );

    debug_string("[maint] begin_next_acd_test stream initiated");

    return 1;
}

void check_acd_test(uint8_t peer_id, const struct access_token_message * msg) {
    if (msg->type == ACCESS_TOKEN_CREATE_RESPONSE) {
        if (msg->create_response.status !=
            (ACD_TESTS[TEST_NUMBER].permissions | ACCESS_TYPE_VALID)) {

            debug_string("Token");
            unsigned int i;
            for (i = 0; i < ACCESS_TOKEN_SIZE; i++) {
                debug_uint8(msg->create_response.token[i]);
            }
            debug_string("Status");
            debug_uint8(msg->create_response.status);
            debug_string("Expected");
            debug_uint8(ACD_TESTS[TEST_NUMBER].permissions | ACCESS_TYPE_VALID);
            
            trigger_test_failure(0x10 | TEST_NUMBER);
        }
        else {
            trigger_test_success(0x10 | TEST_NUMBER);

            struct access_token_message verification_request;
            access_token_create_verification_request(
                &verification_request,
                msg->create_response.token
            );

            stream_set_data_outgoing(
                ACD_ADDRESS,
                &verification_request,
                sizeof(struct access_token_message)
            );

            bus_proto_initiate_stream(
                &ACD_PROTO_STATE,
                ACD_ADDRESS,
                stream_get_data_outgoing(ACD_ADDRESS),
                sizeof(struct access_token_message)
            );
        }
    }
    else if (msg->type == ACCESS_TOKEN_VERIFICATION_RESPONSE) {
        if (msg->verification_response.access_type !=
            (ACD_TESTS[TEST_NUMBER].permissions | ACCESS_TYPE_VALID)) {
            debug_string("[maint] Got");
            debug_uint8(msg->verification_response.access_type);
            debug_string("[maint] Expected");
            debug_uint8(ACD_TESTS[TEST_NUMBER].permissions | ACCESS_TYPE_VALID);
            trigger_test_failure(0x18 | TEST_NUMBER);
        }
        else {
            trigger_test_success(0x18 | TEST_NUMBER);
            TEST_NUMBER += 1;
            begin_next_acd_test();
        }
    }
    else {
        trigger_test_failure(0x1f);
    }
}

void acd_test_run_bus() {
    // debug_string("[maint] acd_test_run_bus");
    if (bus_proto_flush_blocking_send_data(&ACD_PROTO_STATE) == 0) {
        if (READ_DATA_READY(MAINT_BUS)) {
            // debug_string("[maint] read data is ready on maint bus. First two bytes:");
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[0]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[1]);

            bus_proto_process(
                &ACD_PROTO_STATE,
                (void *) DEVICE_READ_BUFFER(MAINT_BUS),
                *DEVICE_READ_SIZE(MAINT_BUS)
            );

            READ_DATA_COMPLETE_SET(MAINT_BUS, 1)
            READ_DATA_READY_SET(MAINT_BUS, 0)
        }
        else {
            // debug_string("[maint] No read data ready on maint bus");
            bus_proto_process(&ACD_PROTO_STATE, NULL, 0);
        }
    }
    else {
        // debug_string("[maint] bus_proto_flush_blocking_send_data returned non-zero");
    }
}