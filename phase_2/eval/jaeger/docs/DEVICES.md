# Devices

A diagram of the various devices, and their bus connections, is provided below:

```
                                   PriVileged Bus
++----------------+------+------------------------+
 |                |      |
 |                |      |         Maintenance Bus
+--+-------+---------------+----------------------+
 | |       |      |      | |
 | |       |      |      | |       RF Comms Bus
+-------------------+--------+--------------------+
 | |       |      | |    | | |
 | |       |      | |    | | |
 | |       |      | |    | | |
++-+--+ +--+--+ +-+-+-+ ++-+-++
| ACD | | MCD | | RCD | | FSD |
+-----+ +-----+ +-----+ +-----+
```

## Access Control Device

### System Configuration

* **Protocol Address**: 1
* **Connected Busses**: Privileged Bus, Maintenance Bus
* **Available Permissions**: None (N/A)

### System Overview

The access control devices prevents unauthorized interactions between devices in the JS-DOFFS. It implements an authentication system, whereby a device can request a token from the ACD, and then use this token in requests to other devices. This token can be used to verify a requesting device has the correct permissions, without revealing the key of the requesting device.

All communications with the ACD take place on the Privileged Bus, except those communications between the ACD and the MCD, which take place on the Maintenance Bus.

The following permissions exist:

* `ACCESS_TYPE_FILESYSTEM` - Required to read/write files to the persistent data store.
* `ACCESS_TYPE_DEBUG` - Provides special debug operations to read/write memory, used for development and manufacturer-level maintenance.
* `ACCESS_TYPE_FLIGHT_CONTROLS` - Provides access to the flight controls and sensors. Can query and manipulate controls and sensors.
* `ACCESS_TYPE_COMMUNICATIONS` - Can send and receive messages via the communications system.
* `ACCESS_TYPE_MAINTENANCE` - Can perform device reprogramming.

## Maintenance Control Device

### System Configuration

* **Protocol Address**: 2
* **Connected Busses**: Maintenance Bus
* **Available Permissions**: All Permissions

### System Overview

The maintenance device performs on-system checks to ensure the proper functioning of all systems on JS-DOFSS. These checks take place on a separate, isolated bus known as the maintenance bus. Further information about the functioning of the MCD, and its associated checks, can be found in JS-DOFSS MCD TM rev 3, and is not in scope for this document.

## RF Communications Device (RCD)

### System Configuration

* **Protocol Address**: 3
* **Connected Busses**: Privileged Bus, RF Comms Bus
* **Available Permissions**: `ACCESS_TYPE_COMMUNICATIONS`

### System Overview

The RF Communications Device (RCD) provides programmatic off-platform access to some of the busses on the JS-DOFSS. For this demonstration, we have removed the RF Device and replaced it with a TCP/IP connection for testing purposes. While the RCD has the ability to both read and write messages to its connected busses, the primary purpose of the RCD is the enablement of off-platform bus monitoring for safety and maintenance purposes, as well as emergency systems control if required.

## File Storage Device (FSD)

### System Configuration

* **Protocol Address**: 4
* **Connected Busses**: Privileged Bus, Maintenance Bus, RF Comms Bus
* **Available Permissions**: `ACCESS_TYPE_COMMUNICATIONS`, `ACCESS_TYPE_FILESYSTEM`

### System Overview

The File Storage Device provides authenticated access to sensitive files stored on the Jaeger System. All requests to the FSD require a token with the `ACCESS_TYPE_FILESYSTEM` permission. A basic flow chart of the FSD's functioning is provided below:

```


                                +------------------+
                                | Request Received |
                                +--------+---------+
                                         |
                                         v
                                 +-------+--------+  Invalid token  +----------------+
                                 | Validate Token +---------------->+ Return Failure |
                                 +-------+--------+                 +----------------+
                                         |
                                         | Valid Token
                                         v
                            +------------+-----------+
              +-------------+ Determine Request Type +------------+
              |             +-----------+------------+            |
              |                         |                         |
              | File Exists             | File Size               | File Contents
              |                         |                         |
              v                         v                         v
    +---------+----------+    +---------+----------+   +----------+-------------+
    | Does File Exist?   |    | Query Successful?  |   | Query Successful       |
    ++-----------+-------+    +-+------------------+   +-+-+--------------------+
     |           |            | |                        | |
     | Yes       | No         | | Yes                    | | Yes
     |           |            | | +-------------------+  | | +-----------------------------+
     v           v            | | | Return 1          |  | | | Return 1                    |
+----+-----+  +--+-------+    | +>+ Size field set to |  | | | Up to MAX bytes placed in   |
| Return 1 |  | Return 0 |    |   |   file size       |  | +>+   contents                  |
+----------+  +----------+    |   +-------------------+  |   | size set to number of bytes |
                              |                          |   |   placed in contents        |
                              | No                       |   +-----------------------------+
                              |   +----------+           |
                              +-->+ Return 0 |           | No
                                  +----------+           |   +----------+
                                                         +-->+ Return 0 |
                                                             +----------+

```
