#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "result.h"
#include "system.h"

int main (int argc, char * argv[]) {
    
    struct system_configuration configuration = {
        .vms = {
            {
                .vm_enabled = 1,
                .program_path = "devices/test-0-d0",
                .device_configurations = {
                    {
                        .device_enabled = 1,
                        .device_type = DEVICE_TYPE_BUS,
                        .bus_num = 0
                    },
                    { .device_enabled = 0 },
                    { .device_enabled = 0 },
                    { .device_enabled = 0 }
                }
            },
            {
                .vm_enabled = 1,
                .program_path = "devices/test-0-d1",
                .device_configurations = {
                    {
                        .device_enabled = 1,
                        .device_type = DEVICE_TYPE_BUS,
                        .bus_num = 0
                    },
                    { .device_enabled = 0 },
                    { .device_enabled = 0 },
                    { .device_enabled = 0 }
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

    for (i = 0; i < 32; i++) {
        LOG_INFO("System Tick");
        system_tick(system);
    }

    system_delete(system);

    return 0;
}