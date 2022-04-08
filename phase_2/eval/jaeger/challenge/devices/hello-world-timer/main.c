#include "platform/platform.h"


void interrupt_handler();
void output_a_character();
void reset_timer_interrupt();


int char_place;
const char * string = "Hello World\n";

void output_a_character() {
    if (string[char_place] != 0) {
        char * stdout = (char *) 0x80000000;
        *stdout = string[char_place];
        char_place++;
    }
}

void interrupt_handler() {
    uint32_t mcause = csr_mcause_get();

    if (mcause == R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT) {
        output_a_character();
        reset_timer_interrupt();
    }
}

void reset_timer_interrupt() {
    uint64_t new_interrupt_time = *MTIME_PTR + (10000 / TICK_MICROSECOND_SCALE);
    *MTIMECMP_PTR = new_interrupt_time;
}

void main () {
    set_interrupt_handler(&interrupt_handler);
    reset_timer_interrupt();

    uint32_t mie = csr_mie_get();
    mie |= R5_CSR_MIE_MTIE;
    csr_mie_set(mie);

    uint32_t mstatus = csr_mstatus_get();
    mstatus |= R5_CSR_MSTATUS_MIE;
    csr_mstatus_set(mstatus);

    wait_for_interrupt();
}