#ifndef challenge_HEADER
#define challenge_HEADER

#include "platform/platform.h"

/*******************************************************************************
 * SHARED BUS PROTOCOL INFORMATION
 ******************************************************************************/

#define ACD_ADDRESS 1
#define MCD_ADDRESS 2
#define RCD_ADDRESS 3
#define FSD_ADDRESS 4

/*******************************************************************************
 * ACCESS CONTROL DEVICE PROTOCOL
 ******************************************************************************/

/*
    ACCESS_TYPE is an 8-bit field.
    
    The lower 7 bits are masked flags, corresponding to different permissions
    devices can have on the bus.
    
    The high bit is used in the access bus protocol.
    * 1 means the access is valid
    * 0 means the access is not valid
*/

#define ACCESS_TYPE_FILESYSTEM 1
#define ACCESS_TYPE_DEBUG 2
#define ACCESS_TYPE_COMMUNICATIONS 16
#define ACCESS_TYPE_MAINTENANCE 32

#define ACCESS_TYPE_PERMISSIONS_MASK 0x7f

#define ACCESS_TYPE_VALID 0x80

#define ACCESS_TYPE_ALL ( \
    ACCESS_TYPE_FILESYSTEM | ACCESS_TYPE_DEBUG | \
    ACCESS_TYPE_COMMUNICATIONS | ACCESS_TYPE_MAINTENANCE \
)

#define ACCESS_TOKEN_KEY_SIZE 16
#define ACCESS_TOKEN_SIZE 8

enum access_token_message_type {
    ACCESS_TOKEN_CREATE_REQUEST        = 0,
    ACCESS_TOKEN_CREATE_RESPONSE       = 1,
    ACCESS_TOKEN_VERIFICATION_REQUEST  = 2,
    ACCESS_TOKEN_VERIFICATION_RESPONSE = 3
};

struct access_token_create_request {
    uint8_t access_type;
    uint8_t key[ACCESS_TOKEN_KEY_SIZE];
}__attribute__((__packed__));

struct access_token_create_response {
    uint8_t status;
    uint8_t token[ACCESS_TOKEN_SIZE];
}__attribute__((__packed__));

struct access_token_verification_request {
    uint8_t token[ACCESS_TOKEN_SIZE];
}__attribute__((__packed__));

struct access_token_verification_response {
    uint8_t access_type;
    uint8_t token[ACCESS_TOKEN_SIZE];
}__attribute__((__packed__));

struct access_token_message {
    uint8_t type; // of type enum access_token_message_type
    union {
        struct access_token_create_request create_request;
        struct access_token_create_response create_response;
        struct access_token_verification_request verification_request;
        struct access_token_verification_response verification_response;
    };
}__attribute__((__packed__));


/*******************************************************************************
 * ACCESS CONTROL KEYS AND PERMISSIONS
 * Keys need to be embedded in several devices. Also, permissions are shared
 * between the ACD and maintenance device for testing. They're centralized here.
 ******************************************************************************/

#define MCD_KEY { \
    0xd4, 0xda, 0x05, 0xa7, 0x5a, 0x5b, 0xdc, 0x44, \
    0xe8, 0x2f, 0x08, 0x18, 0x60, 0xdb, 0x88, 0xf0 \
}

#define MCD_PERMISSIONS ACCESS_TYPE_ALL

#define RCD_KEY { \
    0x0f, 0xa2, 0xd0, 0x4f, 0x57, 0x31, 0x0b, 0x8b, \
    0xdd, 0x70, 0x4e, 0xc8, 0xcc, 0xb8, 0x6c, 0xb2 \
}

#define RCD_PERMISSIONS ACCESS_TYPE_COMMUNICATIONS

#define FSD_KEY { \
    0xa6, 0x05, 0x86, 0x4d, 0x28, 0x4f, 0x28, 0x29, \
    0x2a, 0x80, 0xc7, 0x73, 0x78, 0xf4, 0xa3, 0x11 \
}

#define FSD_PERMISSIONS ACCESS_TYPE_FILESYSTEM | ACCESS_TYPE_COMMUNICATIONS


/*******************************************************************************
 * FILE STORAGE PROTOCOL
 ******************************************************************************/

#define FSD_FILENAME_SIZE 0x80
#define FSD_CONTENTS_SIZE 0x40

enum FSD_PROT_TYPE {
    FSD_PROT_FILE_EXISTS_REQUEST    = 1,
    FSD_PROT_FILE_SIZE_REQUEST      = 2,
    FSD_PROT_FILE_CONTENTS_REQUEST  = 3,
    FSD_PROT_FILE_EXISTS_RESPONSE   = 4,
    FSD_PROT_FILE_SIZE_RESPONSE     = 5,
    FSD_PROT_FILE_CONTENTS_RESPONSE = 6
};

struct fsd_request_file_exists {
    char filename[FSD_FILENAME_SIZE];
}__attribute__((__packed__));

struct fsd_request_file_size {
    char filename[FSD_FILENAME_SIZE];
}__attribute__((__packed__));

struct fsd_request_file_contents {
    uint32_t offset;
    uint32_t size;
    uint8_t filename[FSD_FILENAME_SIZE];
}__attribute__((__packed__));

struct fsd_request {
    uint8_t type; // Of type enum FSD_PROT_TYPE
    uint8_t token[ACCESS_TOKEN_SIZE];
    union {
        struct fsd_request_file_exists file_exists;
        struct fsd_request_file_size file_size;
        struct fsd_request_file_contents file_contents;
    };
}__attribute__((__packed__));

struct fsd_response_file_exists {
    uint8_t success;
}__attribute__((__packed__));

struct fsd_response_file_size {
    uint8_t success;
    uint32_t size;
}__attribute__((__packed__));

struct fsd_response_file_contents {
    uint8_t success;
    uint32_t offset;
    uint32_t size;
    uint8_t contents[FSD_CONTENTS_SIZE];
}__attribute__((__packed__));

struct fsd_response {
    uint8_t type; // Of type FSD_PROT_TYPE
    union {
        struct fsd_response_file_exists file_exists;
        struct fsd_response_file_size file_size;
        struct fsd_response_file_contents file_contents;
    };
}__attribute__((__packed__));

#endif