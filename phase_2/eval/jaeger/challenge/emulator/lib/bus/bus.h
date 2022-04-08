#ifndef bus_HEADER
#define bus_HEADER

#include <stdint.h>

#include "rc.h"


#define BUS_BUFFER_SIZE 64


struct bus_connection {
    struct bus_connection * next;
    struct bus_connection * prev;
    struct bus * bus;
    struct rc rc;
    uint32_t head;
    uint32_t tail;
    uint8_t data[BUS_BUFFER_SIZE];
};


struct bus_connection * bus_connection_create(struct bus * bus);
int bus_connection_ref(struct bus_connection * bus_connection);
int bus_connection_unref(struct bus_connection * bus_connection);
void bus_connection_delete(struct bus_connection *);

/**
* Add data to this bus connection's internal buffer.
* @param bus_connection
* @param data The data to add to the internal buffer
* @param data_len The length of data in bytes
*/
int bus_connection_append(
    struct bus_connection * bus_connection,
    const uint8_t * data,
    uint32_t data_len
);

/**
* Send data to all the other connections on the bus
* @param bus_connection
* @param data The data to add to the internal buffer
* @param data_len The length of data in bytes
*/
int bus_connection_send(
    struct bus_connection * bus_connection,
    const uint8_t * data,
    uint32_t data_len
);

/**
 * Consume up to data_len bytes from the bus connection.
 * @param bus_connection
 * @param data An allocated buffer where the data will be placed
 * @param data_len A pointer to a uint32_t holding the size of data. The number
 *                 of bytes consumed will be placed in data_len
 */
int bus_connection_consume(
    struct bus_connection * bus_connection,
    uint8_t * data,
    uint32_t * data_len
);



struct bus {
    struct bus_connection * connections;
};


struct bus * bus_create();
void bus_delete(struct bus * bus);

struct bus_connection * bus_create_connection(struct bus * bus);

int bus_append(struct bus * bus, const uint8_t * data, uint32_t data_len);

#endif