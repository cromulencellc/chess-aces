#include "bus_device.h"

#include <assert.h>

#include "log.h"

struct bus_device * bus_device_create(struct bus_connection * bus_connection) {
    struct bus_device * bus_device = malloc(sizeof(struct bus_device));
    if (bus_device == NULL) {
        return NULL;
    }

    bus_device->device.delete = (device_delete_f) bus_device_delete;
    bus_device->device.run = (device_run_f) bus_device_run;
    bus_device->device.read_u8 = (device_read_u8_f) bus_device_read_u8;
    bus_device->device.write_u8 = (device_write_u8_f) bus_device_write_u8;

    bus_device->bus_connection = bus_connection;
    bus_connection_ref(bus_connection);

    bus_device->status_byte = 0;
    bus_device->read_size = 0;
    bus_device->write_size = 0;
    
    return bus_device;
}


void bus_device_delete(struct bus_device * bus_device) {
    bus_connection_unref(bus_device->bus_connection);
    free(bus_device);
}


enum device_run_result bus_device_run(
    struct bus_device * bus_device,
    struct vm * vm    
) {
    if (BUS_DEVICE_GET_READ_DATA_READY_BIT(bus_device) == 0) {
        uint32_t bytes_read = 16;
        bus_connection_consume(
            bus_device->bus_connection,
            bus_device->read_buffer,
            &bytes_read
        );

        bus_device->read_size = bytes_read;
        if (bytes_read > 0) {
            LOG_TRACEB("vm read %u bytes from bus", bytes_read);
            BUS_DEVICE_SET_READ_DATA_READY_BIT(bus_device, 1);
            BUS_DEVICE_SET_READ_DATA_COMPLETE_BIT(bus_device, 0);
        }
    }

    if (BUS_DEVICE_GET_WRITE_DATA_READY_BIT(bus_device) == 1) {
        bus_connection_send(
            bus_device->bus_connection,
            bus_device->write_buffer,
            bus_device->write_size
        );

        BUS_DEVICE_SET_WRITE_DATA_COMPLETE_BIT(bus_device, 1);
        BUS_DEVICE_SET_WRITE_DATA_READY_BIT(bus_device, 0);
    }

    LOG_TRACEB(
        "bus_device=%p STATUS=0x%02x READ_SIZE=%u WRITE_SIZE=%u",
        bus_device,
        bus_device->status_byte,
        bus_device->read_size,
        bus_device->write_size
    );
    LOG_TRACEB(
        "READ_READY=%u READ_COMPLETE=%u WRITE_READY=%u WRITE_COMPLETE=%u",
        BUS_DEVICE_GET_READ_DATA_READY_BIT(bus_device),
        BUS_DEVICE_GET_READ_DATA_COMPLETE_BIT(bus_device),
        BUS_DEVICE_GET_WRITE_DATA_READY_BIT(bus_device),
        BUS_DEVICE_GET_WRITE_DATA_COMPLETE_BIT(bus_device)
    );

    return DEVICE_OK;
}


int bus_device_read_u8(
    struct bus_device * bus_device,
    uint32_t address,
    uint8_t * value
) {
    if (address == BUS_DEVICE_STATUS_BYTE_OFFSET) {
        LOG_TRACEB("status_byte=0x%x", bus_device->status_byte);
        *value = bus_device->status_byte;
    }
    else if (address == BUS_DEVICE_READ_SIZE_OFFSET) {
        LOG_TRACEB("read_size=0x%x", bus_device->read_size);
        *value = bus_device->read_size;
    }
    else if (address == BUS_DEVICE_WRITE_SIZE_OFFSET) {
        LOG_TRACEB("write_size=0x%x", bus_device->write_size);
        *value = bus_device->write_size;
    }
    else if (    (address >= BUS_DEVICE_READ_BUFFER_OFFSET)
              && (address < (BUS_DEVICE_READ_BUFFER_OFFSET +
                             BUS_DEVICE_READ_BUFFER_SIZE))
    ) {
        *value =
            bus_device->read_buffer[address - BUS_DEVICE_READ_BUFFER_OFFSET];
        LOG_TRACEB(
            "read_buffer[0x%x]=0x%x",
            address - BUS_DEVICE_READ_BUFFER_OFFSET,
            *value
        );
    }
    else if (    (address >= BUS_DEVICE_WRITE_BUFFER_OFFSET)
              && (address < (BUS_DEVICE_WRITE_BUFFER_OFFSET +
                             BUS_DEVICE_WRITE_BUFFER_SIZE))
    ) {
        *value =
            bus_device->write_buffer[address - BUS_DEVICE_WRITE_BUFFER_OFFSET];
        LOG_TRACEB(
            "write_buffer[0x%x] = 0x%x",
            address - BUS_DEVICE_WRITE_BUFFER_OFFSET,
            *value
        );
    }
    else {
        LOG_WARN("address=0x%x", address);
        return -1;
    }

    return 0;
}


int bus_device_write_u8(
    struct bus_device * bus_device,
    uint32_t address,
    uint8_t value
) {
    if (address == BUS_DEVICE_STATUS_BYTE_OFFSET) {
        LOG_TRACEB("status_byte=0x%x", value);
        bus_device->status_byte = value;
    }
    else if (address == BUS_DEVICE_READ_SIZE_OFFSET) {
        LOG_TRACEB("read_size=0x%x", value);
        bus_device->read_size = value;
    }
    else if (address == BUS_DEVICE_WRITE_SIZE_OFFSET) {
        LOG_TRACEB("write_size=0x%x", value);
        bus_device->write_size = value;
    }
    else if (    (address >= BUS_DEVICE_READ_BUFFER_OFFSET)
              && (address < (BUS_DEVICE_READ_BUFFER_OFFSET +
                             BUS_DEVICE_READ_BUFFER_SIZE))
    ) {
        LOG_TRACEB(
            "read_buffer[0x%x]=0x%x",
            address - BUS_DEVICE_READ_BUFFER_OFFSET,
            value
        );
        bus_device->read_buffer[address - BUS_DEVICE_READ_BUFFER_OFFSET] =
            value;
    }
    else if (    (address >= BUS_DEVICE_WRITE_BUFFER_OFFSET)
              && (address < (BUS_DEVICE_WRITE_BUFFER_OFFSET +
                             BUS_DEVICE_WRITE_BUFFER_SIZE))
    ) {
        LOG_TRACEB(
            "write_buffer[0x%x] = 0x%x",
            address - BUS_DEVICE_WRITE_BUFFER_OFFSET,
            value
        );
        bus_device->write_buffer[address - BUS_DEVICE_WRITE_BUFFER_OFFSET] =
            value;
    }
    else {
        LOG_WARN("address=0x%x", address);
        return -1;
    }

    return 0;
}