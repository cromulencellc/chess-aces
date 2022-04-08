#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "devices/bus_device.h"
#include "devices/debug_device.h"
#include "devices/dummy_device.h"
#include "devices/file_storage_device.h"
#include "devices/serial_tcp_device.h"
#include "devices/test_device.h"
#include "log.h"

struct system * system_create(
    const struct system_configuration * configuration
) {
    struct system * system = malloc(sizeof(struct system));

    if (system == NULL) {
        LOG_WARN("malloc fail for system");
        return NULL;
    }

    memset(system, 0, sizeof(struct system));

    /* Create all the busses */
    unsigned int i;
    for (i = 0; i < SYSTEM_NUM_BUSSES; i++) {
        system->busses[i] = bus_create();
        LOG_TRACE("system->busses[%u] = %p", i, system->busses[i]);
        if (system->busses[i] == NULL) {
            LOG_WARN("bus_create() returned NULL");
            return NULL;
        }
    }

    /* Create the VMs */
    for (i = 0; i < SYSTEM_MAX_VMS; i++) {
        if (configuration->vms[i].vm_enabled == 0) {
            LOG_INFO("vm %u is not enabled, skipping", i);
            system->vms[i] = NULL;
            continue;
        }

        LOG_INFO("vm %u is enabled, %s", i, configuration->vms[i].program_path);

        /* Create the VM */
        struct vm * vm = vm_from_elf(configuration->vms[i].program_path);

        if (vm == NULL) {
            LOG_WARN("vm_from_elf returned NULL");
            return NULL;
        }

        /* Add the vm's devices to it */
        unsigned int j;
        for (j = 0; j < VM_MAX_DEVICES; j++) {
            const struct device_configuration * device_configuration;
            device_configuration =
                &configuration->vms[i].device_configurations[j];
            
            if (device_configuration->device_enabled == 0) {
                continue;
            }

            switch (device_configuration->device_type) {
                case DEVICE_TYPE_BUS: {
                    struct bus_connection * bus_connection =
                        bus_create_connection(
                            system->busses[device_configuration->bus_num]
                        );
                    struct bus_device * bus_device =
                        bus_device_create(bus_connection);
                    vm_add_device(vm, (struct device *) bus_device);
                    LOG_INFO("Added device type bus");
                    break;
                }
                case DEVICE_TYPE_DEBUG: {
                    struct debug_device * debug_device = debug_device_create();
                    vm_add_device(vm, (struct device *) debug_device);
                    LOG_INFO("Added device type debug");
                    break;
                }
                case DEVICE_TYPE_DUMMY: {
                    struct dummy_device * dummy_device = dummy_device_create();
                    vm_add_device(vm, (struct device *) dummy_device);
                    LOG_INFO("Added device type dummy");
                    break;
                }
                case DEVICE_TYPE_FILE_STORAGE: {
                    struct file_storage_device * file_storage_device =
                        file_storage_device_create();
                    vm_add_device(vm, (struct device *) file_storage_device);
                    LOG_INFO("Added device type file storage");
                    break;
                }
                case DEVICE_TYPE_TEST: {
                    struct test_device * test_device = test_device_create();
                    vm_add_device(vm, (struct device *) test_device);
                    LOG_INFO("Added device type test");
                    break;
                }
                case DEVICE_TYPE_SERIAL: {
                    const char * port_str = getenv("PORT");
                    if (port_str == NULL) {
                        LOG_ERROR("Missing PORT environment variable");
                        return NULL;
                    }
                    struct serial_tcp_device * serial_tcp_device =
                        serial_tcp_device_accept(port_str);
                    if (serial_tcp_device == NULL) {
                        LOG_ERROR("Error creating serial tcp device");
                        return NULL;
                    }
                    vm_add_device(vm, (struct device *) serial_tcp_device);
                    LOG_INFO("Added device type serial");
                    break;
                }
                default: {
                    LOG_WARN(
                        "Invalid bus type for vm %u device %u (%u)",
                        i,
                        j,
                        device_configuration->device_type
                    );
                    return NULL;
                }
            }
        }

        LOG_SUCCESS(
            "vm %u (%s) successfully initialized",
            i,
            configuration->vms[i].program_path
        );
        system->vms[i] = vm;
    }

    return system;
}

void system_delete(struct system * system) {
    unsigned int i;

    LOG_TRACE();

    for (i = 0; i < SYSTEM_NUM_BUSSES; i++) {
        if (system->busses[i] != NULL) {
            LOG_TRACE("bus_delete(system->busses[%u]", i);
            bus_delete(system->busses[i]);
        }
    }

    for (i = 0; i < SYSTEM_MAX_VMS; i++) {
        if (system->vms[i] != NULL) {
            vm_delete(system->vms[i]);
        }
    }

    free(system);
}

void system_tick(struct system * system) {
    unsigned int i;

    for (i = 0; i < SYSTEM_MAX_VMS; i++) {
        if (system->vms[i] == NULL) {
            continue;
        }

        LOG_TRACEW(
            "vm %u, mtime=%lx, mtimecmp=%lx",
            i,
            system->vms[i]->mtime,
            system->vms[i]->mtimecmp
        );


        if (vm_run_devices(system->vms[i])) {
            LOG_WARN("Got error running devices for vm %u", i);
        }
        unsigned int j;
        for (j = 0; j < 5000; j++) {
            enum result result = vm_step(system->vms[i]);
            if (result != OK) {
                if (result == OK_WAITING_FOR_INTERRUPT) {
                    LOG_TRACE("vm %u is waiting for interrupt step %u", i, j);
                    break;
                }
                else {
                    LOG_WARN(
                        "Got error %s for vm %u",
                        result_string(result),
                        i
                    );
                    break;
                }
            }
        }

        vm_timer_tick(system->vms[i]);
    }

    struct timespec timespec;
    timespec.tv_sec = 0;
    timespec.tv_nsec = 5 * 1000 * 1000;
    nanosleep(&timespec, NULL);
}