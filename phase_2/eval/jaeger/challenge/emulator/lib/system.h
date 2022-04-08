#ifndef system_HEADER
#define system_HEADER


#include "bus/bus.h"
#include "devices/device.h"
#include "vm/vm.h"


enum device_type {
    DEVICE_TYPE_BUS,
    DEVICE_TYPE_DEBUG,
    DEVICE_TYPE_DUMMY,
    DEVICE_TYPE_FILE_STORAGE,
    DEVICE_TYPE_SERIAL,
    DEVICE_TYPE_TEST
};


struct device_configuration {
    /** When set to 1, this device is enabled */
    unsigned int device_enabled;
    /** What type of device is this */
    enum device_type device_type;
    /** If this is a bus, which bus is this */
    unsigned int bus_num;
};


struct vm_configuration {
    unsigned int vm_enabled;
    const char * program_path;
    struct device_configuration device_configurations[VM_MAX_DEVICES];
};

#define SYSTEM_MAX_VMS 4

struct system_configuration {
    struct vm_configuration vms[SYSTEM_MAX_VMS];
};


#define SYSTEM_NUM_BUSSES 8

struct system {
    struct bus * busses[SYSTEM_NUM_BUSSES];
    struct vm * vms[SYSTEM_MAX_VMS];
};


struct system * system_create(
    const struct system_configuration * configuration
);

void system_delete(struct system * system);

void system_tick(struct system * system);


#endif