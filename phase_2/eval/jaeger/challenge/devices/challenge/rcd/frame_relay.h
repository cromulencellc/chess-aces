#ifndef frame_relay_HEADER
#define frame_relay_HEADER

#include "platform/platform.h"
#include "platform/prot.h"

#define CACHED_DATA_SIZE 256

struct frame_relay {
    uint8_t cached_data[CACHED_DATA_SIZE];
    unsigned int cached_data_len;

    union {
        struct frame incoming_frame;
        uint8_t incoming_frame_data[sizeof(struct frame)];
    };
    unsigned int incoming_frame_len;
};

void frame_relay_init(struct frame_relay * frame_relay);

/** Process the incoming data into the frame_relay state
 * 
 * This can be called with NULL data, which will advance data in the cache to
 * process additional frames.
 * 
 * @param frame_relay a properly initializaed frame_relay
 * @param data Raw data off the bus, or NULL if we want to process the cache
 * @param data_len Size of data in bytes
 * @return 0 if there are no pending frames
 *         1 if there is a pending frame
 *         -1 if something terrible has happened
 */
int frame_relay_process_data(
    struct frame_relay * frame_relay,
    const uint8_t * data,
    unsigned int data_len
);


/** Returns a pointer to the frame. Any subsequent call to modify the
 * frame_relay will invalidate the frame.
 * @param frame_relay A frame_relay that has just returned 1 on a call to
 *                    frame_relay_process_data
 * @return A pointer to the frame, or NULL if the frame is not ready to be
 *         consumed.
 */
const struct frame * frame_relay_consume(struct frame_relay * frame_relay);

#endif