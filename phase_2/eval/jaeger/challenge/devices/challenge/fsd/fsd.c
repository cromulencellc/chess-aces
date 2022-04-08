#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "callbacks.h"
#include "challenge.h"
#include "debug.h"
#include "fsd.h"
#include "peer.h"


struct bus_proto_state PRIV_BUS_PROTO_STATE;
struct bus_proto_state RCD_BUS_PROTO_STATE;


void main () {
    /* Initialize bus protocol state machines */
    set_interrupt_handler(&interrupt_handler);
    reset_timer_interrupt();

    bus_proto_state_initialize(
        &PRIV_BUS_PROTO_STATE,
        FSD_ADDRESS,
        &priv_bus_callbacks
    );

    bus_proto_state_initialize(
        &RCD_BUS_PROTO_STATE,
        FSD_ADDRESS,
        &rcd_bus_callbacks
    );

    uint32_t mie = csr_mie_get();
    mie |= R5_CSR_MIE_MTIE;
    csr_mie_set(mie);

    uint32_t mstatus = csr_mstatus_get();
    mstatus |= R5_CSR_MSTATUS_MIE;
    csr_mstatus_set(mstatus);

    debug_string("[fsd] Initialization Complete");

    wait_for_interrupt();
}


/* Handle incoming bus traffic */
void run_bus() {
    if (bus_proto_flush_blocking_send_data(&PRIV_BUS_PROTO_STATE) == 0) {
        if (READ_DATA_READY(PRIV_BUS)) {
            bus_proto_process(
                &PRIV_BUS_PROTO_STATE,
                (void *) DEVICE_READ_BUFFER(PRIV_BUS),
                *DEVICE_READ_SIZE(PRIV_BUS)
            );

            READ_DATA_COMPLETE_SET(PRIV_BUS, 1)
            READ_DATA_READY_SET(PRIV_BUS, 0)
        }
        else {
            bus_proto_process(&PRIV_BUS_PROTO_STATE, NULL, 0);
        }
    }
    else {
        debug_string("[fsd] bus_proto_flush_blocking_send_data returned non-zero");
    }

    if (bus_proto_flush_blocking_send_data(&RCD_BUS_PROTO_STATE) == 0) {
        if (READ_DATA_READY(RCD_BUS)) {
            bus_proto_process(
                &RCD_BUS_PROTO_STATE,
                (void *) DEVICE_READ_BUFFER(RCD_BUS),
                *DEVICE_READ_SIZE(RCD_BUS)
            );

            READ_DATA_COMPLETE_SET(RCD_BUS, 1)
            READ_DATA_READY_SET(RCD_BUS, 0)
        }
        else {
            bus_proto_process(&RCD_BUS_PROTO_STATE, NULL, 0);
        }
    }
    else {
        debug_string("[fsd] bus_proto_flush_blocking_send_data returned non-zero");
    }
    
    process_fsd_peers();
}


void interrupt_handler() {
    uint32_t mcause = csr_mcause_get();

    if (mcause == R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT) {
        run_bus();
        reset_timer_interrupt();
    }
}

void reset_timer_interrupt() {
    uint64_t new_interrupt_time = *MTIME_PTR + (10000 / TICK_MICROSECOND_SCALE);
    *MTIMECMP_PTR = new_interrupt_time;
}
