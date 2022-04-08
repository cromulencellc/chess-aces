#include "platform/platform.h"
#include "platform/stdlib.h"

/*
    This device ready data from the bus, and then echoes that data back onto the
    bus. It is used to test the bus code in concert with test-0-echo-d0.
*/


void interrupt_handler();
void output_a_character();
void reset_timer_interrupt();


uint8_t pending_buffer[READ_BUFFER_SIZE];
uint8_t pending_buffer_len;

volatile uint8_t * BUS_STATUS_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + STATUS_BYTE_OFFSET);
volatile uint8_t * BUS_READ_SIZE_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + READ_SIZE_OFFSET);
volatile uint8_t * BUS_WRITE_SIZE_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + WRITE_SIZE_OFFSET);

volatile uint8_t * BUS_READ_BUFFER =
    (uint8_t *) (DEVICE_0_ADDRESS + READ_BUFFER_OFFSET);
volatile uint8_t * BUS_WRITE_BUFFER =
    (uint8_t *) (DEVICE_0_ADDRESS + WRITE_BUFFER_OFFSET);

void run_bus() {
    /* Can we write to the bus? */
    if (    ((*BUS_STATUS_PTR & WRITE_DATA_READY_BIT) == 0)
         /* And we have data to write to the bus */
         && (pending_buffer_len > 0)) {
        
        memcpy((void *) BUS_WRITE_BUFFER, pending_buffer, pending_buffer_len);
        *BUS_WRITE_SIZE_PTR = pending_buffer_len;
        *BUS_STATUS_PTR &= ~WRITE_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR |= WRITE_DATA_READY_BIT;

        pending_buffer_len = 0;
    }

    /* Do we have incoming data ready? */
    if (*BUS_STATUS_PTR & READ_DATA_READY_BIT) {
        memcpy(pending_buffer, (void *) BUS_READ_BUFFER, *BUS_READ_SIZE_PTR);
        pending_buffer_len = *BUS_READ_SIZE_PTR;

        *BUS_STATUS_PTR |= READ_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR &= ~READ_DATA_READY_BIT;
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

void main () {
    pending_buffer_len = 0;

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