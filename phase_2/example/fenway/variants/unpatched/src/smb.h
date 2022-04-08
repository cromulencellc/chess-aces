#ifndef SMB_H
#define SMB_H
#include "pgm.h"
#include "ppm.h"
#include <stdint.h>
typedef struct __attribute__((packed)) {
  uint8_t protocol[4];
  uint8_t command[1];
  uint32_t error_status[1];
  uint8_t flags1[1];
  uint16_t flags2[1];
  uint16_t PIDhigh[1];
  uint16_t reserved[1];
  uint16_t TID[1];
  uint16_t PIDlow[1];
  uint16_t UID[1];
  uint16_t MID[1];
} Req_Header;
typedef struct __attribute__((packed)) {
  uint8_t word_count[1];
  uint8_t *words;
} Parameter_Block;
typedef struct __attribute__((packed)) {
  uint16_t byte_count[1];
  char *bytes;
} Data_Block;
typedef struct __attribute__((packed)) {
  Req_Header header;
  Parameter_Block pblock;
  Data_Block dblock;
} SMB_Struct;
SMB_Struct *SMB_parse(int main_accept);
void print_req_header(Req_Header rh);
void print_param_block(Parameter_Block pb);
void print_data_block(Data_Block db);
void print_smb_struct(SMB_Struct *smb);
#endif
