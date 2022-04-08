#ifndef acd2_HEADER
#define acd2_HEADER

#include "platform/prot.h"
#include "challenge.h"

extern struct bus_proto_state BUS_PRIV_PROTO_STATE;
extern struct bus_proto_state BUS_MAINT_PROTO_STATE;

#define PRIV_BUS DEVICE_0_ADDRESS
#define MAINT_BUS DEVICE_1_ADDRESS

void main();

void acd2_run_bus();

void acd2_interrupt_handler();

void acd2_reset_timer_interrupt();

int acd2_process_request(
    struct bus_proto_state* bus_proto_state,
    uint8_t peer_id,
    const struct access_token_message *
);

#endif