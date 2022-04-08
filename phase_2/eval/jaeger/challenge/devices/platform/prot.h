#ifndef prot_HEADER
#define prot_HEADER

#include "platform.h"

struct frame_address {
    uint8_t to: 4;
    uint8_t from: 4;
} __attribute__((__packed__));

#define FRAME_DATA_MAXLEN 6

struct frame {
    struct frame_address address;
    uint8_t flags;
    union {
        uint32_t length;
        uint8_t data[FRAME_DATA_MAXLEN];
    };
} __attribute__((__packed__));


#define FRAME_SIZE (sizeof(struct frame))
#define FRAME_HEADER_SIZE 2


/** A callback for when we receive an abort frame. */
typedef int (* abort_callback_f)(const struct frame *);

/** A callback for when we receive a single data frame. */
typedef int (* data_callback_f)(const struct frame *);

/** A callback for when a stream is initialized.
*
* If the callback will accept the frame, it sets data_address and returns 0.
* Otherwise, the callback returns non-zero.
*/
typedef int (* stream_initiate_callback_f)(
    const struct frame *,
    void ** data_address
);

/** A callback for when a stream is completed. */
typedef void (* stream_complete_callback_f)(const struct frame *);

/** A callback for when we receive a broadcast frame. */
typedef int (* broadcast_callback_f)(const struct frame *);

/** A callback used to send data on the bus.
* 
* The callback should look at the frame to determine its length, and
* appropriately write the frame to the bus.
*
* Returns 1 if the frame was sent.
* Returns 0 if the frame could not yet be sent.
* Returns any other value on error.
*/
typedef int (* send_callback_f)(const struct frame *);

struct bus_proto_callbacks {
    abort_callback_f abort_callback;
    data_callback_f data_callback;
    stream_initiate_callback_f stream_initiate_callback;
    stream_complete_callback_f stream_complete_callback;
    broadcast_callback_f broadcast_callback;
    send_callback_f send_callback;
};


#define PROT_FLAGS_TYPE_MASK                0b11100000

#define PROT_FLAGS_TYPE_ABORT               0b11100000
#define PROT_FLAGS_TYPE_BROADCAST           0b10000000
#define PROT_FLAGS_TYPE_STREAM              0b01000000
#define PROT_FLAGS_TYPE_DATA                0b00100000
#define PROT_FLAGS_TYPE_STREAM_ACK          0b01100000

#define PROT_FLAGS_STREAM_END               0b00010000
#define PROT_FLAGS_STREAM_INITIATE          0b00001000
#define PROT_FLAGS_DATA_SIZE_MASK           0b00000111

#define PROT_DATA_SIZE(XXX) ((XXX) & PROT_FLAGS_DATA_SIZE_MASK)

#define PROT_BROADCAST_ADDRESS              0xf

#define PROT_MAX_PEERS 15

#define MAX_BLOCKING_SEND_FRAMES 8

enum peer_state {
    /* Peer is ready for sending/receiving. */
    PEER_STATE_READY,
    /* We are receiving, and we are awaiting more data */
    PEER_STATE_RECEIVING_STREAM,
    /* We are sending, and we are awaiting an ack */
    PEER_STATE_AWAITING_ACK,
    /* We are sending, and we are awaiting the final ack */
    PEER_STATE_SENDING_STREAM_COMPLETED
};

struct bus_peer {
    enum peer_state state;
    uint8_t * data_ptr;
    uint32_t stream_size_total;
    uint32_t stream_size_transmitted;
};


int bus_peer_reset(struct bus_peer * peer);


#define BUS_PROTO_STATE_BUF_SIZE (sizeof(struct frame) * 8)

struct bus_proto_state {
    /* The state between us and all the peers on the bus */
    struct bus_peer peers[PROT_MAX_PEERS];

    /* Our address on the bus */
    uint8_t this_address;

    /* All of our callbacks for various actions. */
    const struct bus_proto_callbacks * callbacks;

    /* We may need to buffer bytes from the buf before we can process them. */
    union {
        struct frame frame;
        uint8_t buf[BUS_PROTO_STATE_BUF_SIZE];
    };

    /* The number of bytes in buf. */
    uint32_t buf_size;

    /* If we are blocking on frames we need to send, these are the frames. */
    struct frame blocking_send_frames[8];

    /* The number of frames in the blocking_send_frames array */
    unsigned int num_blocking_send_frames;
};


/**
 * Initialize the bus protocol stack.
 * @param bus_proto_state Allocated, unitialized memory to hold the state of
 *      this buf protocol stack.
 * @param this_address The address of this device on the bus.
 */
int bus_proto_state_initialize(
    struct bus_proto_state * bus_proto_state,
    uint8_t this_address,
    const struct bus_proto_callbacks * callbacks
);

/**
 * Runs the given data over the bus protocol stack. If we given data_len of 0,
 * this will also attempt to send any pending messages.
 * @param bus_proto_state An initialized bus protocol stack
 * @param data Data we have received from the bust
 * @param data_len The length of data in bytes
 */
int bus_proto_process(
    struct bus_proto_state * bus_proto_state,
    const uint8_t * data,
    uint32_t data_len
);

/**
 * Initiate a stream to the given peer.
 * 
 * This will initiate the stream. However, the peer may deny the stream. If this
 * happens, the abort_callback will be called.
 * 
 * Upon completion of the stream, the stream_completion_callback will be called
 * with the final ack frame.
 * @param bus_proto_state An initialized bus protocol stack.
 * @param peer_address The address of the peer on the bus this data should be
 *                     sent to.
 * @param data A pointer to the data we want to send.
 * @param data_len The length of the data in bytes.
 * @return 0 If the stream can be initiated.
 *         -1 if the stream cannot be initiated.
 */
int bus_proto_initiate_stream(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_id,
    const uint8_t * data,
    uint32_t data_len
);

/**
 * Send a single frame of data to the given peer
 * @param bus_proto_state An initialized bus protocol stack.
 * @param peer_address The address of the peer on the bus this data should be
 *                     sent to.
 * @param data A pointer to the data we want to send.
 * @param data_len The length of the data in bytes.
 * @return 0 If the stream can be initiated.
 *         -1 if the stream cannot be initiated.
 */
int bus_proto_send_data(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_id,
    const uint8_t * data,
    uint32_t data_len
);

/**
 * Sends data over the bus, using the bus's send callback.
 * @param bus_proto_state An initialized bus protocol stack
 * @param frame The frame to send
 */
int bus_proto_send(
    struct bus_proto_state * bus_proto_state,
    const struct frame * frame
);

/**
 * If we have pending data in the bus protocol stack, attempt to send that data.
 * @param bus_proto_state An initialized bus protocol stack
 * @return 0 if there is no pending data to send when this call completes.
 *         1 if there is still pending data to send.
*/
int bus_proto_flush_blocking_send_data(struct bus_proto_state *);

/**
 * Sends an abort message to the given peer.
 * @param bus_proto_state An initialized bus protocol stack
 * @param peer_address The address of the peer we are sending an abort to
 */
int bus_proto_abort(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_address
);

/**
 * @param bus_proto_state An initialized bus protocol stack
 * @return 1 if there are pending send frames, 0 otherwise
 */
int bus_proto_has_pending_send_frames(struct bus_proto_state * bus_proto_state);

/**
 * @param bus_proto_state An initialized bus protocol stack
 * @param peer_id The id of the peer for which you want the state
 * @return The current state of the peer.
 */
enum peer_state bus_proto_peer_state(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_id
);

/*
    All devices have an address from 0-14. Address 15 is reserved as a broadcast
    address.

    Devices can broadcast messages up to 6-bytes in length by issuing a
    broadcast message to the bus. Devices can also communicate directly with one
    another. Devices can send up to 6 bytes directly to another device by
    sending a single data packet. Data above 6 bytes requires a stream.

    Stream initialization:
        The requester sends a frame with the flags:
            PROT_FLAGS_TYPE_REQUEST_STREAM | PROT_FLAGS_STREAM_INITIATE
        The data field is 4-bytes, little endian, and holes the length of data
        the device wishes to stream.
    
        If the responder is willing to accept the stream, the responder responds
        with:
            PROT_FLAGS_TYPE_REQUEST_STREAM_ACK | PROT_FLAGS_STREAM_INITIAZTE
        
        Otherwise, the responder will responder with ABORT.
        
        The requester then sends the data with the flags:
            PROT_FLAGS_TYPE_REQUEST_STREAM
        
        Each frame is acknowledged by the requester.

        The last frame is sent with flags:
            PROT_FLAGS_TYPE_REQUEST_STREAM | PROT_FLAGS_STREAM_END
        
        And is acknowledged with:
            PROT_FLAGS_TYPE_RESPONSE_STREAM | PROT_FLAGS_STREAM_END

    The size bits of flag hold the size of the data portion of the frame.
*/

#endif