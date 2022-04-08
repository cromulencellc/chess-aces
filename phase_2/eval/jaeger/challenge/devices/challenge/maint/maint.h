#ifndef maint_HEADER
#define maint_HEADER

#include "platform/prot.h"
#include "challenge.h"

extern struct bus_proto_state BUS_MAINT_PROTO_STATE;

#define MAINT_BUS DEVICE_1_ADDRESS
#define TEST_DEVICE DEVICE_2_ADDRESS

void main();

void run_bus();

void interrupt_handler();

void reset_timer_interrupt();

int process_request(uint8_t peer_id, const struct access_token_message *);

void trigger_test_failure(uint8_t cause);

void trigger_test_success(uint8_t cause);

#endif