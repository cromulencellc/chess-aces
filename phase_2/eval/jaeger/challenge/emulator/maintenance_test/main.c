#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "result.h"
#include "system.h"


#define PRIV_BUS 0
#define FCWS_BUS 1
#define MAINT_BUS 2
#define RFCOMM_BUS 3
#define UTILITY_BUS 4


int main (int argc, char * argv[]) {
    
    struct system_configuration configuration = {
        .vms = {
            {
                .vm_enabled = 1,
                .program_path = "devices/acd2",
                .device_configurations = {
                    {
                        .device_enabled = 1,
                        .device_type = DEVICE_TYPE_BUS,
                        .bus_num = PRIV_BUS
                    },
                    {
                        .device_enabled = 1,
                        .device_type = DEVICE_TYPE_BUS,
                        .bus_num = MAINT_BUS
                    },
                    { .device_enabled = 1, .device_type = DEVICE_TYPE_DUMMY },
                    { .device_enabled = 1, .device_type = DEVICE_TYPE_DEBUG },
                }
            },
            {
                .vm_enabled = 1,
                .program_path = "devices/maint",
                .device_configurations = {
                    // {
                    //     .device_enabled = 1,
                    //     .device_type = DEVICE_TYPE_BUS,
                    //     .bus_num = PRIV_BUS
                    // },
                    { .device_enabled = 1, .device_type = DEVICE_TYPE_DUMMY },
                    {
                        .device_enabled = 1,
                        .device_type = DEVICE_TYPE_BUS,
                        .bus_num = MAINT_BUS
                    },
                    { .device_enabled = 1, .device_type = DEVICE_TYPE_TEST },
                    { .device_enabled = 1, .device_type = DEVICE_TYPE_DEBUG },
                }
            },
            { .vm_enabled = 0 },
            { .vm_enabled = 0 }
        }
    };
    
    struct system * system = system_create(&configuration);

    if (system == NULL) {
        LOG_ERROR("Failed to create system");
        return -1;
    }

    unsigned int i;

    for (i = 0; i < 1000; i++) {
        LOG_TRACE("System Tick");
        system_tick(system);
    }

    LOG_INFO("Done executing");

    system_delete(system);

    return 0;
}