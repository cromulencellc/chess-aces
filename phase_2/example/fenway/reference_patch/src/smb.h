#ifndef SMB_H
#define SMB_H

#include <stdint.h>
#include "pgm.h"
#include "ppm.h"

/*
 The Req_Header of the SMB Packet
*/
typedef struct __attribute__((packed)){
  
  //4 Byte String Literal  
  //'\xFF' , S , M , B
  //S, M, and B in ASCII
  uint8_t protocol[4];
  
  //One Byte Commnd identifer
  uint8_t command[1];

  uint32_t error_status[1];

  //8 bit, bit-field for special attribute of request
  uint8_t flags1[1];

  //16-bit, bit-field for special attributes of request
  uint16_t flags2[1];

  //Upper 16 bits of PID from client
  uint16_t PIDhigh[1];

  //RESERVED
  uint16_t reserved[1];

  //The number used to identify a resource of a share or printer
  uint16_t TID[1];

  //Lower 16 bits of PID from client
  uint16_t PIDlow[1];

  //The User ID user identifer from client
  uint16_t UID[1];

  //Creates logical seperation in actions from client
  uint16_t MID[1];

} Req_Header;


/*
 The parameter block contains "Words" which
 are seen as parameters to the smb "function"
 that is being called
 */
typedef struct __attribute__((packed)){

  uint8_t word_count[1];

  //Must be 2 * wordcount
  uint8_t *words;

} Parameter_Block;

typedef struct __attribute__((packed)){
  
  uint16_t byte_count[1];
  
  //Must be the amount of byte count param
  char *bytes;

} Data_Block;

typedef struct __attribute__((packed)){

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
