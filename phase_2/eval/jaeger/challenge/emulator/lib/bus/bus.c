#include "bus.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

struct bus_connection * bus_connection_create(struct bus * bus) {
    struct bus_connection * bus_connection =
        malloc(sizeof(struct bus_connection));

    if (bus_connection == NULL) {
        return NULL;
    }

    rc_init(
        &bus_connection->rc,
        bus_connection,
        (rc_delete_callback) bus_connection_delete
    );

    bus_connection->next = NULL;
    bus_connection->prev = NULL;
    bus_connection->bus = bus;
    bus_connection->head = 0;
    bus_connection->tail = 0;
    
    return bus_connection;
}

void bus_connection_delete(struct bus_connection * bus_connection) {
    LOG_TRACE();
    if (bus_connection->prev != NULL) {
        bus_connection->prev->next = bus_connection->next;
    }
    else {
        bus_connection->bus->connections = bus_connection->next;
    }

    if (bus_connection->next != NULL) {
        bus_connection->next->prev = bus_connection->prev;
    }

    free(bus_connection);
}

int bus_connection_ref(struct bus_connection * bus_connection) {
    return rc_ref(&bus_connection->rc);
}

int bus_connection_unref(struct bus_connection * bus_connection) {
    LOG_TRACE();
    return rc_unref(&bus_connection->rc);
}

int bus_connection_append(
    struct bus_connection * bus_connection,
    const uint8_t * data,
    uint32_t data_len
) {
    uint32_t offset = 0;

    LOG_TRACEC("Appending data, len=%u to %p", data_len, bus_connection);

    while (offset < data_len) {
        /* Buffer is full */
        if (bus_connection->tail == bus_connection->head - 1) {
            /* BUG
            * This will cause the bus buffer to silently drop data. We should
            * probably throw an error here.
            */
            break;
        }

        /* Tail is at the end of the buffer, send it to the beginning. */
        else if (bus_connection->tail == BUS_BUFFER_SIZE) {
            bus_connection->tail = 0;
            continue;
        }

        bus_connection->data[bus_connection->tail] = data[offset];
        bus_connection->tail++;
        offset++;
    }
    
    return 0;
}

int bus_connection_send(
    struct bus_connection * bus_connection,
    const uint8_t * data,
    uint32_t data_len
) {
    struct bus_connection * bc;

    LOG_TRACEC("Sending data, len=%u", data_len);

    for (bc = bus_connection->bus->connections; bc != NULL; bc = bc->next) {
        if (bc == bus_connection) {
            continue;
        }
        bus_connection_append(bc, data, data_len);
    }

    return 0;
}

int bus_connection_consume(
    struct bus_connection * bus_connection,
    uint8_t * data,
    uint32_t * data_len
) {
    uint32_t offset = 0;

    LOG_TRACEB(
        "bus_connection = %p, head=0x%x, tail=0x%x",
        bus_connection,
        bus_connection->head,
        bus_connection->tail
    );

    while (offset < *data_len) {
        if (bus_connection->head == bus_connection->tail) {
            break;
        }

        if (bus_connection->head == BUS_BUFFER_SIZE) {
            bus_connection->head = 0;
            continue;
        }

        data[offset] = bus_connection->data[bus_connection->head];
        bus_connection->head++;
        offset++;
    }

    *data_len = offset;

    if (offset > 0) {
        LOG_TRACEC("Consumed bus data, %u bytes", *data_len);
    }

    return 0;
}



struct bus * bus_create() {
    struct bus * bus = malloc(sizeof(struct bus));

    bus->connections = NULL;

    return bus;
}

void bus_delete(struct bus * bus) {
    LOG_TRACE("bus=%p, bus->connections=%p", bus, bus->connections);
    while (bus->connections != NULL) {
        struct bus_connection * conn = bus->connections;
        bus_connection_unref(bus->connections);
        if (conn == bus->connections) {
            bus->connections = bus->connections->next;
        }
    }

    free(bus);
}

struct bus_connection * bus_create_connection(struct bus * bus) {
    struct bus_connection * bus_connection = bus_connection_create(bus);
    if (bus_connection == NULL) {
        LOG_WARN("bus_connection_create returned NULL");
        return NULL;
    }

    if (bus->connections != NULL) {
        LOG_TRACEC("Existing connection, adding to list");
        bus->connections->prev = bus_connection;
        bus_connection->next = bus->connections;
    }

    bus->connections = bus_connection;

    #ifdef LOG_LEVEL_TRACE
    unsigned int num_connections = 0;
    struct bus_connection * bc;
    for (bc = bus->connections; bc != NULL; bc = bc->next) {
        num_connections++;
    }

    LOG_TRACEC(
        "Created bus_connection=%p, total=%u, bus=%p",
        bus_connection,
        num_connections,
        bus
    );
    #endif

    return bus_connection;
}

int bus_append(struct bus * bus, const uint8_t * data, uint32_t data_len) {
    struct bus_connection * bus_connection;

    #ifdef TRACE_LEVEL_DEBUG
    unsigned int buf_len = 256;
    char buf[256];
    char * buf_ptr = buf;

    unsigned int i;
    unsigned int bytes_to_encode = data_len < 16 ? data_len : 16;

    for (i = 0; i < bytes_to_encode; i++) {
        int bytes_encoded = snprintf(buf, buf_len, "%02X ", data[i]);
        buf_ptr = (char *) ((uintptr_t) buf_ptr + bytes_encoded);
        buf_len -= bytes_encoded;
    }

    LOG_DEBUG("bus_append: %s", buf);
    #endif

    for (
        bus_connection = bus->connections;
        bus_connection != NULL;
        bus_connection = bus_connection->next
    ) {
        bus_connection_append(bus_connection, data, data_len);
    }

    return 0;
}