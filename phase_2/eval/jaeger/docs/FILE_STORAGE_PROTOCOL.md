## File Storage Protocol

The File Storage Protocol (FSP) works through stream Bus Protocol messages, and provides interoperability between the FSD and other JS-DOFSS devices. The FSP allows devices with the `ACCESS_TYPE_FILESYSTEM` permission to read sensitive files stored by the FSD.

A JS-DOFSS device can request the existence of files, the size of files, and the contents of files. A typical exchange with the FSD is modelled in the below diagram:

```
+------+                   +---+                         +---+
|Device|                   |FSD|                         |ACD|
+------+                   +-+-+                         +-+-+
   |                         |                             |
   +-------------------------+---------------------------->+
   |              Token Creation Request                   |
   |                         |                             |
   +<------------------------+-----------------------------+
   |              Token Creation Response                  |
   |                         |                             |
   +------------------------>+                             |
   |      FSD Request        |                             |
   |                         |                             |
   |                         +---------------------------->+
   |                         | Token Verification Request  |
   |                         |                             |
   |                         +<----------------------------+
   |                         | Token Verification Response |
   |                         |                             |
   +<------------------------+                             |
   |      FSD Response       |                             |
   +                         +                             +
```

There are six message types implemented by the FSP. They are:

* File Exists Request
* File Exists Response
* File Size Request
* File Size Response
* File Contents Request
* File Contents Response

Each request begins with a 1-byte message type, followed by an 8-byte token. Each response begins with a 1-byte type. The available types are:

```
FSD_PROT_FILE_EXISTS_REQUEST    = 1
FSD_PROT_FILE_SIZE_REQUEST      = 2
FSD_PROT_FILE_CONTENTS_REQUEST  = 3
FSD_PROT_FILE_EXISTS_RESPONSE   = 4
FSD_PROT_FILE_SIZE_RESPONSE     = 5
FSD_PROT_FILE_CONTENTS_RESPONSE = 6
```

This protocol has two constants, `FSD_FILENAME_SIZE` and `FSD_CONTENTS_SIZE`.

```
FSD_FILENAME_SIZE = 0x80
FSD_CONTENTS_SIZE = 0x40
```

### File Exists Request

```
+---------------------+--------------------------+
| 1-byte Request Type | Variable-length filename |
+---------------------+--------------------------+
```

The file exists request begins with the byte `FSD_PROT_FILE_EXISTS_REQUEST`, `0x01`, followed by a null-terminated string up to `0x80` bytes in length. If the filename is not null-terminated, the FSD will null-terminate the filename for you.

### File Exists Response

```
+----------------------+-----------------+
| 1-byte Response Type | 1-byte Response |
+----------------------+-----------------+
```

The file exists response begins with the byte `FSD_PROT_FILE_EXISTS_RESPONSE`, `0x02`, followed by a single byte which indicates whether the file exists. If the response byte is `0x01`, the file exists. If the response byte is `0x00`, the file does not exist.

### File Size Request

```
+---------------------+--------------------------+
| 1-byte Request Type | Variable-length filename |
+---------------------+--------------------------+
```

The file exists request begins with the byte `FSD_PROT_FILE_SIZE_REQUEST`, `0x03`, followed by a null-terminated string up to `0x80` bytes in length. If the filename is not null-terminated, the FSD will null-terminate the filename for you.

### File Size Response

```
+----------------------+----------------+--------------+
| 1-byte Response Type | 1-byte Success | 4-byte Size |
+----------------------+----------------+--------------+
```

The file size response begins with the byte `FSD_PROT_FILE_SIZE_RESPONSE`, `0x04`, followed by a 1-byte success value, followed by the file size in a 32-bit little-endian field. If the success value is `1`, the `size` field contains the size of the requested file. If the success value is `0`, the `size` field is undefined.

### File Contents Request

```
+---------------------+---------------+-------------+--------------------------+
| 1-byte Request Type | 4-byte Offset | 4-byte Size | Variable-length filename |
+---------------------+---------------+-------------+--------------------------+
```

The file contents request begins with a 1-byte request type, followed by a 4-byte little-endian offset from the beginning file, followed by a 4-byte little-endian size field for the number of bytes requested, and trailed by a null-terminated filename with a max-length of `0x80` bytes. If the filename is not null-terminated, the FCD will null-terminate the filename for you.

This request is meant to be sent multiple times, requesting subsequent chunks of a file, until the entire file has been fetched from the FSD.

The maximum value for the `size` field is `0x40` bytes. If `size` is greater than `0x40` bytes, the File Storage Device will truncate the response to `0x40` bytes.

### File Contents Response

```
+----------------------+----------------+---------------+-------------+
| 1-byte Response Type | 1-byte Success | 4-byte Offset | 4-byte Size |
+----------------------+---+------------+---------------+-------------+
| Variable-length contents |
+--------------------------+
```

The file contents response begins with a 1-byte response type, followed by a 1-byte success field, followed by a 4-byte little endian offset from the beginning of the file, followed by a 4-byte little-endian size field for the number of bytes returned in contents, followed by a variable-length array containing file contents. The number of bytes returned in the `contents` field is determined by the `size` field, and will not exceed `FSD_CONTENTS_SIZE`, which is set to `0x40`.

If the success field is `1`, the other field in this response type are valid. If the response type is `0`, `offset` should be `0`, `size` should be `0`, and `contents` is undefined.
