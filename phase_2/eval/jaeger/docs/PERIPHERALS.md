# Peripherals

The JS-DOFSS is responsible for mapping perhipherals into device memory. The JS-DOFSS supports the mapping of up to 8 peripherals per device. Devices are mapped into device memory beginning at address `0x40000000`, and increment by `0x00010000` bytes per device.

```
DEVICE 0 ADDRESS: 0x40000000
DEVICE 1 ADDRESS: 0x40010000
DEVICE 2 ADDRESS: 0x40020000
DEVICE 3 ADDRESS: 0x40030000
DEVICE 4 ADDRESS: 0x40040000
DEVICE 5 ADDRESS: 0x40050000
DEVICE 6 ADDRESS: 0x40060000
DEVICE 7 ADDRESS: 0x40070000
```

Which peripheral type is mapped to which address is dependent on the device.

There are 6 device types. They are:

* Bus Device
* Debug Device
* Dummy Device
* File Storage Device
* Serial TCP Device
* Test Device

Each peripheral serves a unique purpose, and provides a unique memory mapping to a device.

While noted previously, we note again that JS-DOFSS does not yes currently support externally-driven interrupts. All devices run off internal timer-driven interrupts. Devices will poll peripherals for information on a regular interval.

## Bus Peripheral

The bus peripheral allows devices to read and write bytes to common busses. Bytes written to busses can be read from, and written to, by all devices connected to the bus. The bus provides the sole means for inter-device communication.

### Overview

```
+--------+------+--------------+
| Offset | Size | Field        |
+--------+------+--------------+
| 0x0000 | 0x01 | STATUS       |
+--------+------+--------------+
| 0x0001 | 0x01 | READ_SIZE    |
+--------+------+--------------+
| 0x0002 | 0x01 | WRITE_SIZE   |
+--------+------+--------------+ 
| 0x0100 | 0x10 | READ BUFFER  |
+--------+------+--------------+
| 0x0200 | 0x10 | WRITE BUFFER |
+--------+------+--------------+
```

### Functioning

The bus device is controlled through the STATUS bit. The STATUS bit has the following fields:

```
Bit Index:    0 1 2 3 4 5 6 7
Name:         R C W D U U U U

R: READ DATA READY BIT
C: READ DATA COMPLETE BIT
W: WRITE DATA READY BIT
D: WRITE DATA COMPLETE BIT
U: UNUSED
```

* `READ_DATA_READY_BIT`: When this bit is set, the bus peripheral has data available in the read buffer the device should read.
* `READ_DATA_COMPLETE_BIT`: When this bit is set, the device has finished reading data from the read buffer. The device must also set the `READ_DATA_READY_BIT` to `0`.
* `WRITE_DATA_READY_BIT`: When this bit is set, the device has placed data into the `WRITE_BUFFER`, and the bus device should send that data to the bus.
* `WRITE_DATA_COMPLETE_BIT`: When this bit is set, the bus peripheral has written the data from the write buffer to the bus. Also, the bus peripheral must set the `WRITE_DATA_READY_BIT` to `0`.

At all times the `READ_DATA_READY_BIT` and `WRITE_DATA_READY_BIT` are set, the `WRITE_SIZE` and `READ_SIZE` fields be valid for the number of bytes to be read and written from teh `READ_BUFFER` and `WRITE_BUFFER` fields.

## Debug Peripheral

The debug peripheral provides a mechanism for devices to provide debugging information by means of printing strings and integers. The output of debug peripherals should display to `STDOUT` for users of the `JS-DOFSS`.

### Overview

```
+--------+-------+--------------+
| Offset | Size  | Field        |
+--------+-------+--------------+
| 0x0000 | 0x01  | WRITE        |
+--------+-------+--------------+
| 0x0100 | 0x100 | BUF          |
+--------+-------+--------------+
```

Acceptable values for `WRITE` are:

```
SEND_STRING = 1
SEND_UINT8  = 2
SEND_UINT32 = 3
```

### Functioning

A device can write one of three values to the `WRITE` field. The interpretation of the `BUF` field is dependent upon the value of the `WRITE` field.

When the `WRITE` field is set to `STRING`, a null-terminated string will be read from the `BUF` field and written to `STDOUT`.

When the `WRITE` field is set to `UINT8`, a single byte will be read from the `BUF` field and written to `STDOUT`.

When the `WRITE` field is set to `UINT32`, a 32-bit little-endian integer will be read from the `BUF` field and written to `STDOUT`.

## Dummy Peripheral

Certain device libraries require peripherals to be present at certain addresses. A "Dummy Device" is a peripheral for which all reads and writes are invalid, but allows the JS-DOFSS to advance advance peripheral addresses. We can insert dummy devices to force other peripherals to exist at specific addresses.

## File Storage Peripheral

The file storage peripheral provides a device access to the underlying filesystem.

### Overview

```
+--------+-------+--------------+
| Offset | Size  | Field        |
+--------+-------+--------------+
| 0x0000 | 0x01  | ACTION       |
+--------+-------+--------------+
| 0x0004 | 0x01  | SIZE         |
+--------+-------+--------------+
| 0x0008 | 0x04  | OFFSET       |
+--------+-------+--------------+
| 0x0080 | 0x80  | FILENAME     |
+--------+-------+--------------+
| 0x0100 | 0x40  | CONTENTS     |
+--------+-------+--------------+
```

### Functioning

The File Storage Peripheral takes three commands. They are:

```
FSD_ACTION_FILE_EXISTS = 1
FSD_ACTION_FILE_SIZE   = 2
FSD_ACTION_READ_FILE   = 3
```

#### File Exists

The file exists action checks for the existence of a given file.

To use this action, first place the filename of the file you intend to check into the `FILENAME` field as a null-terminated string. Then write the value `0x01` for `FSD_ACTION_FILE_EXISTS` into the `ACTION` register.

The peripheral will cause the device to block until the action is complete. Once complete, the device can read from the `ACTION` register. If the `ACTION` contains 1, the file exists. If the `ACTION` register contains `0`, the file does not exist.

#### File Size

The file size action returns the size of a file.

To use this action, first place the filename of the file you intend to check into the `FILENAME` field as a null-terminated string. Then write the value `0x02` for `FSD_ACTION_FILE_SIZE` into the `ACTION` register.

The peripheral will cause the device to block until the action is complete. Once complete, the device can read from the `ACTION` register. If the `ACTION` register contains `1`, the file size can be read from the `SIZE` field. If the `ACTION` register contains `0`, the peripheral failed to get the size of the requested file.

#### File Size

The file size action returns the size of a file.

To use this action, first place the filename of the file you intend to check into the `FILENAME` field as a null-terminated string. Then write the value `0x02` for `FSD_ACTION_FILE_SIZE` into the `ACTION` register.

The peripheral will cause the device to block until the action is complete. Once complete, the device can read from the `ACTION` register. If the `ACTION` register contains `1`, the file size can be read from the `SIZE` field. If the `ACTION` register contains `0`, the peripheral failed to get the size of the requested file.

#### File Contents

The file contents actions fetches up to 64 bytes from a file at a given offset.

To use this action, first place the filename of the file for which you want contents into the `FILENAME` field as a null-terminated string. Place the number of bytes you wish to read into the `SIZE` field. Place the offset from the beginning of the file where you want the read to take place into the `OFFSET` field. Then write the value `0x03` for `FSD_ACTION_READ_FILE` into the `ACTION` register.

Both the `SIZE` and `OFFSET` fields take 32-bit little-endian values. If the value written to the `SIZE` field is greater than `0x40`, only upto `0x40` bytes will be read.

The peripheral will cause the device to block until the action is complete. Once complete, the device can read from the `ACTION` register.

If the `ACTION` register contains `1`, the requested file contents can be found in the `CONTENTS` field. The `SIZE` field will denote how many bytes were read from the `FILE`. The value of the `CONTENTS` field beyond the bounds denoted by the `SIZE` field are undetermined.

If the `ACTION` register contains `0`, the requested read failed and the value of all other fields is undefined.

## Test Peripheral

The test peripheral allows devices to signal when the system appears to be malfunctioning. The test peripheral is used primarily by the maintenance device during its startup checks.

### Overview

```
+--------+-------+--------------+
| Offset | Size  | Field        |
+--------+-------+--------------+
| 0x0000 | 0x01  | TEST FAIL    |
+--------+-------+--------------+
| 0x0001 | 0x01  | TEST SUCCESS |
+--------+-------+--------------+
```

### Functioning

The test peripheral works very simply.

Any write to the `TEST FAIL` register will send an error message to `STDOUT` and cause the JS-DOFSS to exit.

Any write to the `TEST SUCCESS` register will send a message to `STDOUT`. JS-DOFSS will continue to function normally.