#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "test-1.h"

/*
    This device receives messages from device 1, then sends them to stdout. It
    then tells device 0 to send the next message.
*/


/* Our standard architecture functions */
void interrupt_handler();
void reset_timer_interrupt();

/* IDs of devices on the bus */
#define SENDER_DEVICE_ID 0
#define PIVOT_DEVICE_ID 1
#define STDOUT_DEVICE_ID 2


/* State machine for this device */
enum state {
    /* We are waiting for the PIVOT_DEVICE_ID to initiate a stream */
    STATE_WAITING_TO_RECEIVE,
    /* We are receiving a stream from PIVOT_DEVICE_ID */
    STATE_RECEIVING_STREAM,
    /* We are waiting to send a message to SENDER_DEVICE_ID */
    STATE_WAITING_TO_SEND,
};

enum state STATE = STATE_WAITING_TO_RECEIVE;


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
    unsigned int length;
    uint8_t peer_id;
    uint8_t stream[STREAM_MAX_SIZE];
} STREAM_STATE;

void stream_reset() {
    STREAM_STATE.active = 0;
    STREAM_STATE.length = 0;
    STREAM_STATE.peer_id = STREAM_PEER_ID_NONE;
};

int abort_callback(const struct frame * frame) {
    if (frame->address.from == PIVOT_DEVICE_ID) {
        if (STATE == STATE_RECEIVING_STREAM) {
            debug_string("d2 Pivot device aborted stream");
            STATE = STATE_WAITING_TO_RECEIVE;
        }
    }
    else if (frame->address.from == SENDER_DEVICE_ID) {
        debug_string("d2 Sender device aborted stream");
        STATE = STATE_WAITING_TO_RECEIVE;
    }
    return 0;
}

int data_callback(const struct frame * frame) {
    return -1;
}

int stream_initiate_callback(const struct frame * frame, void ** data_address) {
    if (    (frame->address.from == PIVOT_DEVICE_ID)
         && (frame->length <= STREAM_MAX_SIZE)
         && (STATE == STATE_WAITING_TO_RECEIVE)
    ) {
        debug_string("d2 stream_initiate from PIVOT device");
        stream_reset();
        STREAM_STATE.active = 1;
        STREAM_STATE.peer_id = PIVOT_DEVICE_ID;
        STREAM_STATE.length = frame->length;
        *data_address = STREAM_STATE.stream;
        STATE = STATE_RECEIVING_STREAM;
        return 0;
    }
    return -1;
}

void stream_complete_callback(const struct frame * frame) {
    /* We will hit this when our stream we are sending is complete */
    if (frame->address.from == PIVOT_DEVICE_ID) {
        debug_string("d2 stream_complete from PIVOT");
        debug_string(STREAM_STATE.stream);
        STATE = STATE_WAITING_TO_SEND;
    }
}

int broadcast_callback(const struct frame * frame) {
    /* Ignore all broadcasts. */
    return 0;
}

int send_callback(const struct frame * frame) {
    debug_string("d2 send_callback");
    if ((*BUS_STATUS_PTR & WRITE_DATA_READY_BIT) == 0) {
        memcpy(
            (void *) BUS_WRITE_BUFFER,
            frame,
            2 + (frame->flags & PROT_FLAGS_DATA_SIZE_MASK)
        );
        
        *BUS_WRITE_SIZE_PTR = 2 + (frame->flags & PROT_FLAGS_DATA_SIZE_MASK);

        *BUS_STATUS_PTR &= ~WRITE_DATA_COMPLETE_BIT;
        *BUS_STATUS_PTR |= WRITE_DATA_READY_BIT;
        return 1;
    }
    else {
        debug_string("d2 send_callback returning 0 can't send frame");
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
    uint16_t leet = 0x1337;
    switch (STATE) {
    case STATE_WAITING_TO_SEND:
        if (bus_proto_send_data(
            &bus_0_proto_state,
            SENDER_DEVICE_ID,
            (void *) &leet,
            2
        ) == 0) {
            STATE = STATE_WAITING_TO_RECEIVE;
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
    debug_uint8(STDOUT_DEVICE_ID);

    bus_proto_state_initialize(&bus_0_proto_state, STDOUT_DEVICE_ID, &callbacks);
    stream_reset();

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