#ifndef device_HEADER
#define device_HEADER

#include <stdint.h>

enum device_run_result {
    DEVICE_OK,
    DEVICE_ERROR
};

struct device;
struct vm;

typedef void (* device_delete_f) (struct device *);
typedef enum device_run_result (* device_run_f) (struct device *, struct vm *);
typedef int (* device_read_u8_f) (
    struct device * device,
    uint32_t address,
    uint8_t * value
);
typedef int (* device_write_u8_f) (
    struct device * device,
    uint32_t address,
    uint8_t value
);

struct device {
    device_delete_f delete;
    device_run_f run;
    device_read_u8_f read_u8;
    device_write_u8_f write_u8;
};

#endif