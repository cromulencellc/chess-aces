#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "challenge.h"
#include "debug.h"
#include "frame_relay.h"
#include "rcd.h"


struct frame_relay PRIV_BUS_FRAME_RELAY;
struct frame_relay RCD_BUS_FRAME_RELAY;

uint8_t * current_bus;

/* Points to:
*   - PRIV_BUS_FRAME_REALY
*   - MAINT_BUS_FRAME_RELAY
*   - RCD_BUS_FRAME_RELAY
* based on whichever bus we are relaying.
*/
struct frame_relay * CURRENT_FRAME_RELAY;

struct rcd_state RCD_STATE;


void main () {
    /* Initialize bus protocol state machines */
    set_interrupt_handler(&interrupt_handler);
    reset_timer_interrupt();

    RCD_STATE.relay_bus = BUS_ID_NONE;
    RCD_STATE.relay_enabled = 0;

    frame_relay_init(&PRIV_BUS_FRAME_RELAY);
    frame_relay_init(&RCD_BUS_FRAME_RELAY);

    CURRENT_FRAME_RELAY = NULL;

    uint32_t mie = csr_mie_get();
    mie |= R5_CSR_MIE_MTIE;
    csr_mie_set(mie);

    uint32_t mstatus = csr_mstatus_get();
    mstatus |= R5_CSR_MSTATUS_MIE;
    csr_mstatus_set(mstatus);

    debug_string("[rcd] Initialization complete, status_byte:");
    debug_uint8(*DEVICE_STATUS(SERIAL_PORT));

    wait_for_interrupt();
}


void set_bus_read_complete(uint8_t * bus) {
    READ_DATA_READY_SET(bus, 0);
    READ_DATA_COMPLETE_SET(bus, 1);
}


int process_bus_data(uint8_t * bus) {
    if (RCD_STATE.relay_enabled == 0) {
        set_bus_read_complete(bus);
        return 0;
    }
    if (RCD_STATE.relay_bus == BUS_ID_NONE) {
        set_bus_read_complete(bus);
        return 0;
    }
    if (    (RCD_STATE.relay_bus == BUS_ID_PRIV)
         && (bus != (const uint8_t *) PRIV_BUS)) {
        set_bus_read_complete(bus);
        return 0;
    }
    if (    (RCD_STATE.relay_bus == BUS_ID_RCD)
         && (bus != (const uint8_t *) RCD_BUS)) {
        set_bus_read_complete(bus);
        return 0;
    }

    /* If we have data pending on the current bus, through it into the
       frame_relay */
    if (READ_DATA_READY(bus)) {
        frame_relay_process_data(
            CURRENT_FRAME_RELAY,
            (const uint8_t *) DEVICE_READ_BUFFER(bus),
            *DEVICE_READ_SIZE(bus)
        );
        set_bus_read_complete(bus);
    }
}


/* Handle incoming bus traffic */
void run_bus() {
    /* Process messages from the privileged bus */
    current_bus = (uint8_t *) PRIV_BUS;
    if (READ_DATA_READY(PRIV_BUS)) {
        process_bus_data((uint8_t *) PRIV_BUS);
    }
    
    /* Process messages from the rf comms bus */
    current_bus = (uint8_t *) RCD_BUS;
    if (READ_DATA_READY(RCD_BUS)) {
        process_bus_data((uint8_t *) RCD_BUS);
    }

    /* If we have incoming data from the serial port */
    if (READ_DATA_READY(SERIAL_PORT)) {
        const volatile uint8_t * serial_data = DEVICE_READ_BUFFER(SERIAL_PORT);

        static unsigned int offset = 0;
        while (offset < *DEVICE_READ_SIZE(SERIAL_PORT)) {
            /* Disable rc comms relay */
            if (serial_data[0] == 0xF0) {
                RCD_STATE.relay_enabled = 0;
                // set_bus_read_complete((uint8_t *) SERIAL_PORT);
                offset++;
            }
            /* Set relay to privileged bus */
            else if (serial_data[offset] == 0xF1) {
                CURRENT_FRAME_RELAY = &PRIV_BUS_FRAME_RELAY;
                RCD_STATE.relay_bus = BUS_ID_PRIV;
                RCD_STATE.relay_enabled = 1;
                // set_bus_read_complete((uint8_t *) SERIAL_PORT);
                offset++;
                debug_string("[rcd] privileged bus relay enabled");
            }
            /* Set relay to RF Comms bus */
            else if (serial_data[offset] == 0xF3) {
                CURRENT_FRAME_RELAY = &RCD_BUS_FRAME_RELAY;
                RCD_STATE.relay_bus = BUS_ID_RCD;
                RCD_STATE.relay_enabled = 1;
                // set_bus_read_complete((uint8_t *) SERIAL_PORT);
                offset++;
                debug_string("[rcd] rf comms bus relay enabled");
            }
            /* Relay message from the serial port to currently selected bus */
            else if (serial_data[offset] == 0xA0) {
                /* We'll come back and try again when we can write to the bus. */
                debug_string("[rcd] sending message");
                if (RCD_STATE.relay_enabled) {
                    uint32_t bus_address = 0;
                    switch (RCD_STATE.relay_bus) {
                    case BUS_ID_PRIV: bus_address = PRIV_BUS; break;
                    case BUS_ID_RCD: bus_address = RCD_BUS; break;
                    }
                    /* We'll try this again when the bus is ready. */
                    if (WRITE_DATA_READY(bus_address) != 0) {
                        debug_string("Write data isn't ready.");
                        break;
                    }
                    if (RCD_STATE.relay_bus > 0) {
                        memcpy(
                            (uint8_t *) DEVICE_WRITE_BUFFER(bus_address),
                            (const uint8_t *) &serial_data[offset + 2],
                            serial_data[offset + 1]
                        );
                        *DEVICE_WRITE_SIZE(bus_address) = serial_data[offset + 1];
                        WRITE_DATA_READY_SET(bus_address, 1);
                        WRITE_DATA_COMPLETE_SET(bus_address, 0);
                        // set_bus_read_complete((uint8_t *) SERIAL_PORT);
                    }
                    else {
                        debug_string("We do not have a valid bus address!");
                        /* This is super bad. Just go ahead and reset the
                        incoming buf state */
                        debug_uint8(RCD_STATE.relay_bus);
                        debug_uint8(bus_address);
                        debug_uint8(WRITE_DATA_READY(bus_address));
                        set_bus_read_complete((uint8_t *) SERIAL_PORT);
                    }
                }
                else {
                    debug_string("[rcd] The relay isn't enabled");
                    set_bus_read_complete((uint8_t *) SERIAL_PORT);
                }
                offset += 2 + serial_data[offset + 1];
            }
            else {
                debug_string("[rcd] Invalid command byte:");
                debug_uint8(serial_data[0]);
                offset++;
            }
        }

        if (offset == *DEVICE_READ_SIZE(SERIAL_PORT)) {
            set_bus_read_complete((uint8_t *) SERIAL_PORT);
            offset = 0;
        }
    }
    
    /*
        If we have bus messages that we have captured from the relay bus,
        start streaming them over the serial port.
    */
    const struct frame * frame;
    if (WRITE_DATA_READY(SERIAL_PORT) == 0) {
        if ((frame = frame_relay_consume(CURRENT_FRAME_RELAY)) != NULL) {
            *DEVICE_WRITE_SIZE(SERIAL_PORT) =
                PROT_DATA_SIZE(frame->flags) + FRAME_HEADER_SIZE;

            memcpy(
                (uint8_t *) DEVICE_WRITE_BUFFER(SERIAL_PORT),
                frame,
                PROT_DATA_SIZE(frame->flags) + FRAME_HEADER_SIZE
            );

            WRITE_DATA_COMPLETE_SET(SERIAL_PORT, 0);
            WRITE_DATA_READY_SET(SERIAL_PORT, 1);
        }
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
