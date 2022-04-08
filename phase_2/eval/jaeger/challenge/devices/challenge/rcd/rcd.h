#ifndef rcd_HEADER
#define rcd_HEADER

#include "platform/prot.h"
#include "challenge.h"

extern struct bus_proto_state BUS_PRIV_PROTO_STATE;
extern struct bus_proto_state BUS_RCD_PROTO_STATE;

enum BUS_ID {
    BUS_ID_NONE = 0,
    BUS_ID_PRIV = 1,
    BUS_ID_RCD = 3
};

struct rcd_state {
    enum BUS_ID relay_bus;
    int relay_enabled;
};

extern struct rcd_state RCD_STATE;

#define PRIV_BUS    DEVICE_0_ADDRESS
#define RCD_BUS     DEVICE_2_ADDRESS
#define SERIAL_PORT DEVICE_4_ADDRESS

void main();

void run_bus();

void interrupt_handler();

void reset_timer_interrupt();

int process_request(uint8_t peer_id, const struct access_token_message *);

void trigger_test_failure(uint8_t cause);

void trigger_test_success(uint8_t cause);

#endif