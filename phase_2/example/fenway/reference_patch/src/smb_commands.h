#ifndef SMB_COMMANDS_H
#define SMB_COMMANDS_H

#include <stdint.h>
#include "smb.h"

//Function Definitions
int SMB_COM_CREATE_DIR(int fd, char *dir_name);
int SMB_COM_DELETE_DIR(int fd, char *dir_name);
int SMB_COM_OPEN(int fd, char *path, uint8_t access_mode);
PGM *SMB_COM_CREATE(int fd, char *path, uint64_t version);
int SMB_COM_CLOSE(int fd, int fd_to_close);
int SMB_COM_DELETE(int fd, char *dir_name);
int SMB_COM_RENAME(int fd, char *dir_name, char *change_name);
int SMB_COM_QUERY_INFORMATION(int fd ,char *path);
int SMB_COM_SET_INFORMATION(int fd, char *path);
int SMB_COM_READ(int fd, unsigned int fid);
int SMB_COM_WRITE(int fd, unsigned int fid, char *bytes, unsigned int bytes_to_write);
PPM *SMB_COM_CREATE_NEW(int fd, char *dir_name, uint64_t version);
int SMB_COM_NEGOTIATE(int fd, SMB_Struct *rq, int error_code);
int SMB_COM_SETUP_ANDX(int fd, SMB_Struct *rq, int error_code);

#endif
