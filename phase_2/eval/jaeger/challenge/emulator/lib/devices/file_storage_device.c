#include "file_storage_device.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

struct file_storage_device * file_storage_device_create() {
    struct file_storage_device * file_storage_device =
        malloc(sizeof(struct file_storage_device));
    if (file_storage_device == NULL) {
        return NULL;
    }

    memset(file_storage_device, 0, sizeof(struct file_storage_device));

    file_storage_device->device.delete = (device_delete_f) file_storage_device_delete;
    file_storage_device->device.run = (device_run_f) file_storage_device_run;
    file_storage_device->device.read_u8 = (device_read_u8_f) file_storage_device_read_u8;
    file_storage_device->device.write_u8 = (device_write_u8_f) file_storage_device_write_u8;

    return file_storage_device;
}

void file_storage_device_delete(struct file_storage_device * file_storage_device) {
    free(file_storage_device);
}

enum device_run_result file_storage_device_run(
    struct file_storage_device * file_storage_device,
    struct vm * vm
) {
    return DEVICE_OK;
}

int file_storage_device_read_u8(
    struct file_storage_device * file_storage_device,
    uint32_t address,
    uint8_t * value
) { 
    if (address == FSD_ACTION_OFFSET) {
        *value = file_storage_device->result;
    }
    else if (address == FSD_SIZE_OFFSET) {
        *value = file_storage_device->size & 0xff;
    }
    else if (address == FSD_SIZE_OFFSET + 1) {
        *value = (file_storage_device->size >> 8) & 0xff;
    }
    else if (address == FSD_SIZE_OFFSET + 2) {
        *value = (file_storage_device->size >> 16) & 0xff;
    }
    else if (address == FSD_SIZE_OFFSET + 3) {
        *value = (file_storage_device->size >> 24) & 0xff;
    }
    else if (address == FSD_OFFSET_OFFSET) {
        *value = (file_storage_device->offset) & 0xff;
    }
    else if (address == FSD_OFFSET_OFFSET + 1) {
        *value = (file_storage_device->offset >> 8) & 0xff;
    }
    else if (address == FSD_OFFSET_OFFSET + 2) {
        *value = (file_storage_device->offset >> 16) & 0xff;
    }
    else if (address == FSD_OFFSET_OFFSET + 3) {
        *value = (file_storage_device->offset >> 24) & 0xff;
    }
    else if (    (address >= FSD_FILENAME_OFFSET)
              && (address < (FSD_FILENAME_OFFSET + FSD_FILENAME_SIZE))) {
        uint32_t offset = address - FSD_FILENAME_OFFSET;
        *value = file_storage_device->filename[offset];
    }
    else if (    (address >= FSD_CONTENTS_OFFSET)
              && (address < (FSD_CONTENTS_OFFSET + FSD_CONTENTS_SIZE))) {
        uint32_t offset = address - FSD_CONTENTS_OFFSET;
        *value = file_storage_device->contents[offset];
    }
    return 0;
}

int file_storage_device_write_u8(
    struct file_storage_device * file_storage_device,
    uint32_t address,
    uint8_t value
) {
    if (address == FSD_ACTION_OFFSET) {
        file_storage_device->filename[FSD_FILENAME_SIZE] = 0;
        switch (value) {
        case FSD_ACTION_FILE_EXISTS:
            if (access((const char *) file_storage_device->filename, R_OK) != -1) {
                file_storage_device->result = 1;
            }
            else {
                file_storage_device->result = 0;
            }
            break;
        case FSD_ACTION_FILE_SIZE: {
            FILE * fh = fopen((const char *) file_storage_device->filename, "rb");
            if (fh == NULL) {
                file_storage_device->result = 0;
                break;
            }
            fseek(fh, 0, SEEK_END);
            size_t filesize = ftell(fh);
            fclose(fh);
            file_storage_device->size = filesize;
            file_storage_device->result = 1;
            break;
        }
        case FSD_ACTION_READ_FILE: {
            FILE * fh = fopen((const char *) file_storage_device->filename, "rb");
            if (fh == NULL) {
                LOG_WARN("Failed to open file: %s", file_storage_device->filename);
                file_storage_device->result = 0;
                break;
            }
            if (file_storage_device->size > FSD_CONTENTS_SIZE) {
                file_storage_device->size = FSD_CONTENTS_SIZE;
            }
            fseek(fh, file_storage_device->offset, SEEK_SET);
            size_t bytes_read =
                fread(file_storage_device->contents, 1, file_storage_device->size, fh);
            fclose(fh);
            file_storage_device->size = bytes_read;
            file_storage_device->result = 1;
            break;
        }
        default:
            file_storage_device->result = 0;
            break;
        }
    }
    else if (address == FSD_SIZE_OFFSET) {
        file_storage_device->size = (file_storage_device->size & 0xffffff00);
        file_storage_device->size |= value;
    }
    else if (address == FSD_SIZE_OFFSET + 1) {
        file_storage_device->size = (file_storage_device->size & 0xffff00ff);
        file_storage_device->size |= (value << 8);
    }
    else if (address == FSD_SIZE_OFFSET + 2) {
        file_storage_device->size = (file_storage_device->size & 0xff00ffff);
        file_storage_device->size |= (value << 16);
    }
    else if (address == FSD_SIZE_OFFSET + 3) {
        file_storage_device->size = (file_storage_device->size & 0x00ffffff);
        file_storage_device->size |= (value << 24);
    }
    else if (address == FSD_OFFSET_OFFSET) {
        file_storage_device->offset = (file_storage_device->offset & 0xffffff00);
        file_storage_device->offset |= value;
    }
    else if (address == FSD_OFFSET_OFFSET + 1) {
        file_storage_device->offset = (file_storage_device->offset & 0xffff00ff);
        file_storage_device->offset |= (value << 8);
    }
    else if (address == FSD_OFFSET_OFFSET + 2) {
        file_storage_device->offset = (file_storage_device->offset & 0xff00ffff);
        file_storage_device->offset |= (value << 16);
    }
    else if (address == FSD_OFFSET_OFFSET + 3) {
        file_storage_device->offset = (file_storage_device->offset & 0x00ffffff);
        file_storage_device->offset |= (value << 24);
    }
    else if (    (address >= FSD_FILENAME_OFFSET)
              && (address < (FSD_FILENAME_OFFSET + FSD_FILENAME_SIZE))) {
        uint32_t offset = address - FSD_FILENAME_OFFSET;
        file_storage_device->filename[offset] = value;
    }
    else {
        return -1;
    }
    return 0;
}