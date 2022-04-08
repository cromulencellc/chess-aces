#ifndef file_storage_device_HEADER
#define file_storage_device_HEADER


#include "device.h"


#define FSD_ACTION_FILE_EXISTS 1
#define FSD_ACTION_FILE_SIZE   2
#define FSD_ACTION_READ_FILE   3

#define FSD_ACTION_OFFSET 0

#define FSD_SIZE_OFFSET 0x4
#define FSD_OFFSET_OFFSET 0x8

#define FSD_FILENAME_OFFSET 0x80
#define FSD_FILENAME_SIZE 0x80
#define FSD_CONTENTS_OFFSET 0x100
#define FSD_CONTENTS_SIZE 0x40


struct file_storage_device {
    struct device device;
    uint8_t result;
    uint32_t size;
    uint32_t offset;
    uint8_t filename[FSD_FILENAME_SIZE + 1];
    uint8_t contents[FSD_CONTENTS_SIZE];
};


struct file_storage_device * file_storage_device_create();
void file_storage_device_delete(struct file_storage_device * file_storage_device);

enum device_run_result file_storage_device_run(
    struct file_storage_device * file_storage_device,
    struct vm * vm
);

int file_storage_device_read_u8(
    struct file_storage_device * file_storage_device,
    uint32_t address,
    uint8_t * value
);

int file_storage_device_write_u8(
    struct file_storage_device * file_storage_device,
    uint32_t address,
    uint8_t value
);


#endif