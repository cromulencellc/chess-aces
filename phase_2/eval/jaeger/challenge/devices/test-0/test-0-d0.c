#include "platform/platform.h"

/*
    This device writes to a bus one byte at a time. The other device on the bus
    should echo that byte. They bus will then read that byte off the bus, and
    print it out.
*/


void interrupt_handler();
void output_a_character();
void reset_timer_interrupt();


int char_place;
const char * string = "Hello World\n";

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
         /* And we aren't done sending the string byte by byte */
         && (string[char_place] != 0)) {
        
        BUS_WRITE_BUFFER[0] = string[char_place++];
        *BUS_WRITE_SIZE_PTR = 1;
        *BUS_STATUS_PTR &= ~WRITE_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR |= WRITE_DATA_READY_BIT;
    }

    /* Do we have incoming data ready? */
    if (*BUS_STATUS_PTR & READ_DATA_READY_BIT) {
        unsigned int i;

        char * stdout = (char *) 0x80000000;
        for (i = 0; i < *BUS_READ_SIZE_PTR; i++) {
            *stdout = BUS_READ_BUFFER[i];
        }

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
    char_place = 0;
    
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