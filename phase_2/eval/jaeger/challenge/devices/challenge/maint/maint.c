#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "acd_test.h"
#include "challenge.h"
#include "debug.h"
#include "maint.h"


struct bus_proto_state BUS_MAINT_PROTO_STATE;

enum test_state {
    ACD_TEST,
    TESTS_COMPLETE
};

enum test_state TEST_STATE;


void lib_tests() {
    uint8_t buf_0[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    uint8_t buf_1[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    uint8_t buf_2[] = { 0x01, 0x02, 0x03, 0x04, 0x06 };
    uint8_t buf_3[] = { 0x01, 0x03, 0x03, 0x04, 0x05 };

    if (memcmp(buf_0, buf_1, 5) != 0) {
        trigger_test_failure(0xf0);
    }

    if (memcmp(buf_0, buf_2, 5) == 0) {
        trigger_test_failure(0xf1);
    }

    if (memcmp(buf_0, buf_3, 5) == 0) {
        trigger_test_failure(0xf2);
    }
}


void main () {
    /* Initialize bus protocol state machines */
    set_interrupt_handler(&interrupt_handler);
    reset_timer_interrupt();

    uint32_t mie = csr_mie_get();
    mie |= R5_CSR_MIE_MTIE;
    csr_mie_set(mie);

    uint32_t mstatus = csr_mstatus_get();
    mstatus |= R5_CSR_MSTATUS_MIE;
    csr_mstatus_set(mstatus);

    lib_tests();

    TEST_STATE = ACD_TEST;

    start_acd_tests();

    debug_string("[maint] Initialization complete");

    wait_for_interrupt();
}



/* Handle incoming bus traffic */
void run_bus() {
    switch (TEST_STATE) {
    case ACD_TEST:
        acd_test_run_bus();
        break;
    }
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

void trigger_test_failure(uint8_t cause) {
    ((volatile uint8_t *) TEST_DEVICE)[0] = cause;
}

void trigger_test_success(uint8_t cause) {
    ((volatile uint8_t *) TEST_DEVICE)[1] = cause;
}