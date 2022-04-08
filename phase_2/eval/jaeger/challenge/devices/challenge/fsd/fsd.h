#ifndef rcd_HEADER
#define rcd_HEADER

#include "platform/prot.h"
#include "challenge.h"

extern struct bus_proto_state PRIV_BUS_PROTO_STATE;
extern struct bus_proto_state RCD_BUS_PROTO_STATE;


#define PRIV_BUS    DEVICE_0_ADDRESS
#define FSD_DEVICE  DEVICE_1_ADDRESS
#define RCD_BUS     DEVICE_2_ADDRESS


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

void main();

void run_bus();

void interrupt_handler();

void reset_timer_interrupt();

#endif