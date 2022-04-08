#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "test-1.h"

/*
    This device sends messages to device 1. Device sends the message to device
    2, and device 2 sends the message to stdout.

    We will receive a 2 byte message from device 2, 0x1337, when device 2 has
    received our previous message and is ready for more.
*/

/* Our standard architecture functions */
void interrupt_handler();
void reset_timer_interrupt();

/* IDs of devices on the bus */
#define SENDER_DEVICE_ID 0
#define PIVOT_DEVICE_ID 1
#define STDOUT_DEVICE_ID 2


#define NUM_MESSAGES 4

int current_message = 0;

const char * messages[NUM_MESSAGES] = {
    "You may say I'm a dreamer",
    "But I'm not the only one",
    "I hope some day you'll join us"
    "And write lots of RISC-V devices"
};


/* State machine for this device */
enum state {
    /* We should initiate a stream to send the next message */
    STATE_START_SENDING_NEXT_MESSAGE,
    /* We are currently sending the message to the pivot device */
    STATE_SENDING_MESSAGE,
    /* We are waiting for a message from the stdout device */
    STATE_WAITING,
    /* We have sent all the messages */
    STATE_DONE
};

enum state STATE = STATE_START_SENDING_NEXT_MESSAGE;


/* And our callback declarations for the bus protocol handler */
int abort_callback(const struct frame * frame);
int data_callback(const struct frame * frame);
int stream_initiate_callback(const struct frame * frame, void ** data_address);
void stream_complete_callback(const struct frame * frame);
int broadcast_callback(const struct frame * frame);
int send_callback(const struct frame * frame);

struct bus_proto_callbacks callbacks = {
    .abort_callback = abort_callback,
    .data_callback = data_callback,
    .stream_initiate_callback = stream_initiate_callback,
    .stream_complete_callback = stream_complete_callback,
    .broadcast_callback = broadcast_callback,
    .send_callback = send_callback
};

/* This is our state for bus 0 */
struct bus_proto_state bus_0_proto_state;

/* We are notified of streams, but we need to store and handle them ourself. */
#define STREAM_MAX_SIZE 256
#define STREAM_PEER_ID_NONE 0xff

struct stream_state {
    unsigned int active;
    unsigned int offset;
    uint8_t peer_id;
    uint8_t stream[STREAM_MAX_SIZE];
} STREAM_STATE;

void stream_reset() {
    STREAM_STATE.active = 0;
    STREAM_STATE.offset = 0;
    STREAM_STATE.peer_id = STREAM_PEER_ID_NONE;
};

int stream_append_data(const uint8_t * data, unsigned int len) {
    if (len > (STREAM_MAX_SIZE - STREAM_STATE.offset)) {
        return -1;
    }
    memcpy(&(STREAM_STATE.stream[STREAM_STATE.offset]), data, len);
    STREAM_STATE.offset += len;

    return 0;
}

int abort_callback(const struct frame * frame) {
    if (frame->address.from == PIVOT_DEVICE_ID) {
        debug_string("d0 Pivot device denied our message");
        STATE = STATE_START_SENDING_NEXT_MESSAGE;
    }
    return 0;
}

int data_callback(const struct frame * frame) {
    debug_string("d0 data_callback");
    debug_uint8(frame->address.from);
    debug_uint8(frame->address.to);
    if (frame->address.from == STDOUT_DEVICE_ID) {
        debug_string("Get data from STDOUT device");
        uint16_t value;
        memcpy(&value, frame->data, 2);
        debug_uint32(value);
        if (value == 0x1337) {
            debug_string("value was good");
            STATE = STATE_START_SENDING_NEXT_MESSAGE;
            current_message++;
        }
        else {
            debug_string("value was bad :( sad face");
        }
        return 0;
    }
    return -1;
}

int stream_initiate_callback(const struct frame * frame, void ** data_address) {
    /* We don't handle any streams, so abort */
    return -1;
}

void stream_complete_callback(const struct frame * frame) {
    /* We will hit this when our stream we are sending is complete */
    if (frame->address.from == PIVOT_DEVICE_ID) {
        STATE = STATE_WAITING;
    }
}

int broadcast_callback(const struct frame * frame) {
    /* Ignore all broadcasts. */
    return 0;
}

int send_callback(const struct frame * frame) {
    if ((*BUS_STATUS_PTR & WRITE_DATA_READY_BIT) == 0) {
        memcpy(
            (void *) BUS_WRITE_BUFFER,
            frame,
            2 + (frame->flags & PROT_FLAGS_DATA_SIZE_MASK)
        );

        *BUS_WRITE_SIZE_PTR = 2 + (frame->flags & PROT_FLAGS_DATA_SIZE_MASK);

        *BUS_STATUS_PTR &= ~WRITE_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR |= WRITE_DATA_READY_BIT;
        debug_string("d0 successfully sent frame");
        return 1;
    }
    else {
        debug_string("d0 could not send frame");
        return 0;
    }
}


/* Handle incoming bus traffic */
void run_bus() {
    /* Do we have incoming data ready? */
    if (*BUS_STATUS_PTR & READ_DATA_READY_BIT) {
        unsigned int i;

        bus_proto_process(
            &bus_0_proto_state,
            (void *) BUS_READ_BUFFER,
            *BUS_READ_SIZE_PTR
        );

        *BUS_STATUS_PTR |= READ_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR &= ~READ_DATA_READY_BIT;
    }
    else {
        bus_proto_process(&bus_0_proto_state, NULL, 0);
    }
}


/* Our main execution loop */
void run_loop() {
    switch (STATE) {
    case STATE_START_SENDING_NEXT_MESSAGE:
        if (current_message == NUM_MESSAGES) {
            STATE = STATE_DONE;
            break;
        }

        if (bus_proto_initiate_stream(
            &bus_0_proto_state,
            PIVOT_DEVICE_ID,
            messages[current_message],
            strlen(messages[current_message]) + 1
        ) == 0) {
            STATE = STATE_SENDING_MESSAGE;
            break;
        }

    default:
        break;
    }
}


void interrupt_handler() {
    uint32_t mcause = csr_mcause_get();

    if (mcause == R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT) {
        run_bus();
        run_loop();
        reset_timer_interrupt();
    }
}

void reset_timer_interrupt() {
    uint64_t new_interrupt_time = *MTIME_PTR + (10000 / TICK_MICROSECOND_SCALE);
    *MTIMECMP_PTR = new_interrupt_time;
}

void main () {
    redirect_puts_to_debug_device((void *) DEVICE_1_ADDRESS);

    debug_string("Device startup. Device ID:");
    debug_uint8(SENDER_DEVICE_ID);

    bus_proto_state_initialize(&bus_0_proto_state, SENDER_DEVICE_ID, &callbacks);

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