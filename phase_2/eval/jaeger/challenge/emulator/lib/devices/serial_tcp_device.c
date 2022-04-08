#include "serial_tcp_device.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bus_device.h"
#include "log.h"


struct serial_tcp_device * serial_tcp_device_accept(const char * port) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int one = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one) == -1) {
        LOG_WARN("error on setsockopt SOL_REUSEADDR");
        close (sockfd);
        return NULL;
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen)) {
        LOG_WARN("error on bind");
        close(sockfd);
        return NULL;
    }

    if (listen(sockfd, 1)) {
        LOG_WARN("error on listen");
        close(sockfd);
        return NULL;
    }

    LOG_SUCCESS("Waiting for connection on port %s", port);

    int clientfd = accept(sockfd, NULL, NULL);

    close(sockfd);

    if (fcntl(clientfd, F_SETFL, O_NONBLOCK)) {
        LOG_WARN("error on fcntl F_SETFL O_NONBLOCK");
        close(sockfd);
        return NULL;
    }

    if (clientfd < 0) {
        LOG_WARN("accept returned negative clientfd");
        return NULL;
    }

    struct serial_tcp_device * serial_tcp_device =
        malloc(sizeof(struct serial_tcp_device));
    
    serial_tcp_device->device.delete =
        (device_delete_f) &serial_tcp_device_delete;
    serial_tcp_device->device.run =
        (device_run_f) &serial_tcp_device_run;
    serial_tcp_device->device.read_u8 =
        (device_read_u8_f) &serial_tcp_device_read_u8;
    serial_tcp_device->device.write_u8 =
        (device_write_u8_f) &serial_tcp_device_write_u8;

    serial_tcp_device->sockfd = clientfd;

    serial_tcp_device->status_byte = 0;
    serial_tcp_device->read_size = 0;
    serial_tcp_device->write_size = 0;
    memset(serial_tcp_device->read_buffer, 0, READ_BUFFER_SIZE);
    memset(serial_tcp_device->write_buffer, 0, WRITE_BUFFER_SIZE);

    return serial_tcp_device;
}


void serial_tcp_device_delete(struct serial_tcp_device * serial_tcp_device) {
    close(serial_tcp_device->sockfd);
    free(serial_tcp_device);
}


enum device_run_result serial_tcp_device_run(
    struct serial_tcp_device * serial_tcp_device,
    struct vm * vm
) {
    // Are we good to read?
    if (GET_READ_DATA_READY_BIT(serial_tcp_device) == 0) {
        int bytes_read =
            recv(
                serial_tcp_device->sockfd,
                serial_tcp_device->read_buffer,
                READ_BUFFER_SIZE - serial_tcp_device->read_size,
                0
            );
        
        if ((bytes_read < 0) && (errno != EWOULDBLOCK)) {
            LOG_ERROR("TCP Stream died: %d\n", bytes_read);
            return DEVICE_ERROR;
        }
        else if (bytes_read > 0) {
            serial_tcp_device->read_size = bytes_read;

            SET_READ_DATA_READY_BIT(serial_tcp_device, 1);
            SET_READ_DATA_COMPLETE_BIT(serial_tcp_device, 0);
        }
    }

    // Are we good to write?
    if (GET_WRITE_DATA_READY_BIT(serial_tcp_device)) {
        int bytes_sent =
            send(
                serial_tcp_device->sockfd,
                serial_tcp_device->write_buffer,
                serial_tcp_device->write_size,
                0
            );
        
        if (bytes_sent != serial_tcp_device->write_size) {
            return DEVICE_ERROR;
        }

        SET_WRITE_DATA_COMPLETE_BIT(serial_tcp_device, 1);
        SET_WRITE_DATA_READY_BIT(serial_tcp_device, 0);
    }

    return DEVICE_OK;
}


int serial_tcp_device_read_u8(
    struct serial_tcp_device * serial_tcp_device,
    uint32_t address,
    uint8_t * value
) {
    if (address == STATUS_BYTE_OFFSET) {
        *value = serial_tcp_device->status_byte;
    }
    else if (address == READ_SIZE_OFFSET) {
        *value = serial_tcp_device->read_size;
    }
    else if (address == WRITE_SIZE_OFFSET) {
        *value = serial_tcp_device->write_size;
    }
    else if (    (address >= READ_BUFFER_OFFSET)
              && (address < READ_BUFFER_OFFSET + READ_BUFFER_SIZE)) {
        *value = serial_tcp_device->read_buffer[address - READ_BUFFER_OFFSET];
    }
    else if (    (address >= WRITE_BUFFER_OFFSET)
              && (address < WRITE_BUFFER_OFFSET + WRITE_BUFFER_SIZE)) {
        *value = serial_tcp_device->write_buffer[address + WRITE_BUFFER_OFFSET];
    }
    else {
        return -1;
    }

    return 0;
}


int serial_tcp_device_write_u8(
    struct serial_tcp_device * serial_tcp_device,
    uint32_t address,
    uint8_t value
) {
    if (address == STATUS_BYTE_OFFSET) {
        serial_tcp_device->status_byte = value;
    }
    else if (address == READ_SIZE_OFFSET) {
        serial_tcp_device->read_size = value;
        if (serial_tcp_device->read_size > READ_BUFFER_SIZE) {
            serial_tcp_device->read_size = READ_BUFFER_SIZE;
        }
    }
    else if (address == WRITE_SIZE_OFFSET) {
        serial_tcp_device->write_size = value;
        if (serial_tcp_device->write_size > WRITE_BUFFER_SIZE) {
            serial_tcp_device->write_size = WRITE_BUFFER_SIZE;
        }
    }
    else if (    (address >= READ_BUFFER_OFFSET)
              && (address < READ_BUFFER_OFFSET + READ_BUFFER_SIZE)) {
        serial_tcp_device->read_buffer[address - READ_BUFFER_OFFSET] = value;
    }
    else if (    (address >= WRITE_BUFFER_OFFSET)
              && (address < WRITE_BUFFER_OFFSET + WRITE_BUFFER_SIZE)) {
        serial_tcp_device->write_buffer[address - WRITE_BUFFER_OFFSET] = value;
    }
    else {
        return -1;
    }

    return 0;
}