#ifndef SMB_RESPONSE_H
#define SMB_RESPONSE_H

#define STATUS_OBJECT_PATH_SYNTAX_BAD 0xC000003B
#define STATUS_OBJECT_NAME_INVALID 0XC000033
#define STATUS_OBJECT_PATH_NOT_FOUND 0xC00003A

#define STATUS_ACCESS_DENIED 0xC0000022

#define STATUS_INSUFF_SERVER_RESOURCES 0xC0000205

#define STATUS_OBJECT_NAME_COLLISION 0xC0000035

#define STATUS_INVALID_SMB 0x00010002

#define STATUS_SMB_BAD_TID 0x00050002

#define STATUS_SMB_BAD_UID 0x0005B002

#define STATUS_SMB_BAD_FID 0x00060001

#define STATUS_MEDIA_WRITE_PROTECTED 0xC00000A2

#define STATUS_DATA_ERROR 0xC000003E

#define STATUS_NO_SUCH_FILE 0xC000000F

#define STATUS_OBJECT_PATH_INVALID 0xC0000039

#define STATUS_CANNOT_DELETE 0xC0000121

#define STATUS_DIRECTORY_NOT_EMPTY 0xC0000101 

#define STATUS_TOO_MANY_OPENED_FILES 0x00040001

#define STATUS_FILE_IS_A_DIRECTORY 0xC00000BA

#define STATUS_OS2_INVALID_ACCESS 0x000C0001

#define STATUS_BAD_DEVICE_TYPE 0xC00000CB

#define STATUS_OBJECT_NAME_NOT_FOUND 0xC0000034

#define STATUS_NOT_SAME_DEVICE 0xC00000D4

#define STATUS_NETWORK_ACCESS_DENIED 0xC00000CA

#define STATUS_NO_MEDIA_DEVICE 0xC0000013

#define STATUS_INVALID_PARAMETER 0xC000000D

#define STATUS_DISK_FILL 0xC000007f

#define STATUS_END_OF_FILE 0xC0000011

#define STATUS_NOT_IMPLEMENTED 0xC0000002

#define SUCCESS 0x00000000


#include "smb.h"

SMB_Struct *CREATE_DIR_RESP(int error_code);
SMB_Struct *DELETE_DIR_RESP(int error_code);
SMB_Struct *OPEN_RESP(int error_code, int fd, char *path, uint8_t access_mode);
SMB_Struct *CREATE_RESP(int fd, int error_code);
SMB_Struct *CLOSE_RESP(int error_code);
SMB_Struct *DELETE_RESP(int error_code);
SMB_Struct *RENAME_RESP(int error_code);
SMB_Struct *QUERY_INFORMATION_RESP(struct stat attrib, int error_code);
SMB_Struct *SET_INFORMATION_RESP(int error_code);
SMB_Struct *READ_RESP(int error_code, int bytes_read, char *bytes);
SMB_Struct *WRITE_RESP(int error_code, unsigned short bytes_written);
SMB_Struct *CREATE_NEW_RESP(int error_code, int fid);
SMB_Struct *NEGOTIATE_RESP(SMB_Struct *rq, int error_code);
SMB_Struct *SETUP_ANDX_RESP(SMB_Struct *rq, int error_code);


#endif
