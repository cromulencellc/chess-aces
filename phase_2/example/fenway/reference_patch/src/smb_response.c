#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "smb_response.h"
#include "system_time.h"

#define WORKGROUP_NAME "FENWAY_WORKGROUP"
#define NATIVE_OS "UBUNTU 18.04"
#define NATIVE_LAN_MAN "NT LM 0.12"
#define MAX_BUFFER_SIZE 0x1104


SMB_Struct *CREATE_DIR_RESP(int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[CREATE_DIR_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    switch(error_code){

        case 0:
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr,"[CREATE_DIR_RESP] Path Syntax Invalid\n");
            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_NOT_FOUND;
            fprintf(stderr,"[CREATE_DIR_RESP] Path Does not Exist\n");
            break;

        case 2:
            smb_response->header.error_status[0] = STATUS_INSUFF_SERVER_RESOURCES;
            fprintf(stderr,"[CREATE_DIR_RESP] The server is out of resources\n");
            break;

        case 3:
            smb_response->header.error_status[0] = STATUS_OBJECT_NAME_COLLISION;
            fprintf(stderr,"[CREATE_DIR_RESP] The specified directory already exists\n");
            break;

        case 4:
            smb_response->header.error_status[0] = STATUS_INVALID_SMB;
            fprintf(stderr,"[CREATE_DIR_RESP] Invalid SMB. Not enough parameter bytes were sent\n");
            break;

        case 5:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_TID;
            fprintf(stderr,"[CREATE_DIR_RESP] The TID is no longer valid\n");
            break;

        case 6:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_UID;
            fprintf(stderr,"[CREATE_DIR_RESP] The UID supplied is not knwon to the session, or the user identified by the UID does not have sufficent privileges.\n");
            break;

        case 7:
            smb_response->header.error_status[0] = STATUS_MEDIA_WRITE_PROTECTED;
            fprintf(stderr,"[CREATE_DIR_RESP] Attempt to write a read-only filesystem\n");
            break;

        case 8:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr,"[CREATE_DIR_RESP] Disk I/O\n");
            break;

        case 9:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[CREATE_DIR_RESP] SUCCESS\n");
            break;
    }

    //Set header to null, Set Error Code 
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;

    return smb_response;
}

SMB_Struct *DELETE_DIR_RESP(int error_code){

    SMB_Struct *smb_response= malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[DELETE_DIR_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    switch(error_code){

        case 0:  
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr,"[DELETE_DIR_RESP] Directory was not found\n");
            break;

        case 1:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr,"[DELETE_DIR_RESP] Path Syntax Invalid\n");

        case 2:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_NOT_FOUND;
            fprintf(stderr,"[DELETE_DIR_RESP] Path does not exist\n");
            break;

        case 3:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_INVALID;
            fprintf(stderr,"[DELETE_DIR_RESP] A component of the path-prefix was not a dir\n");
            break;

        case 4:  
            smb_response->header.error_status[0] = STATUS_CANNOT_DELETE;
            fprintf(stderr,"[DELETE_DIR_RESP] The directory is in use\n");
            break;

        case 5:  
            smb_response->header.error_status[0] = STATUS_DIRECTORY_NOT_EMPTY;
            fprintf(stderr,"[DELETE_DIR_RESP] The directory is not empty\n");
            break;

        case 6:  
            smb_response->header.error_status[0] = STATUS_INVALID_SMB;
            fprintf(stderr,"[DELETE_DIR_RESP] Invalid SMB, not enough parameter bytes were set\n");
            break;

        case 7:  
            smb_response->header.error_status[0] = STATUS_SMB_BAD_TID;
            fprintf(stderr,"[DELETE_DIR_RESP] TID is no longer available\n");
            break;

        case 8:  
            smb_response->header.error_status[0] = STATUS_INSUFF_SERVER_RESOURCES;
            fprintf(stderr,"[DELETE_DIR_RESP] The server is out of resources\n");
            break;

        case 9:  
            smb_response->header.error_status[0] = STATUS_SMB_BAD_UID;
            fprintf(stderr,"[DELETE_DIR_RESP] The UID is not known to the session\n");
            break;

        case 10:  
            smb_response->header.error_status[0] = STATUS_MEDIA_WRITE_PROTECTED;
            fprintf(stderr,"[DELETE_DIR_RESP] Attempt to write to read-only file-system\n");
            break;

        case 11:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr,"[DELETE_DIR_RESP] Disk I/O error\n");
            break;

        case 12:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[DELETE_DIR_RESP] SUCCESS\n");
            break;

    }

    //Set header to null
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;

    return smb_response;
}

SMB_Struct *OPEN_RESP(int error_code, int fd, char *path, uint8_t access_mode){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[OPEN_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0001;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr,"[OPEN_RESP] The named file was not found\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);
            
            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }


            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break;

        case 1:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr,"[OPEN_RESP] Path Syntax Invalid\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break; 

        case 2:  
            smb_response->header.error_status[0] = STATUS_TOO_MANY_OPENED_FILES;
            fprintf(stderr,"[OPEN_RESP] Too many open files, no more FIDs available\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break; 

        case 3:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_INVALID;
            fprintf(stderr,"[OPEN_RESP] A component of the path-prefix was not a directory\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break; 


        case 4:  
            smb_response->header.error_status[0] = STATUS_INSUFF_SERVER_RESOURCES;
            fprintf(stderr,"[OPEN_RESP] The server is out of resources\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break; 

        case 5:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr,"[OPEN_RESP] Disk Error I/O\n");

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = 0x00;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            *(smb_response->pblock.words +  2) = 0x00; 
            *(smb_response->pblock.words +  3) = 0x00; 
            *(smb_response->pblock.words +  4) = 0x00; 
            *(smb_response->pblock.words +  5) = 0x00; 

            //AccessMode
            *(smb_response->pblock.words +  6) = 0x00; 
            *(smb_response->pblock.words +  7) = 0x00; 
            break; 

        case 6:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[OPEN_RESP] SUCCESS\n");

            struct stat attrib;
            stat(path, &attrib);

            //Parameters Block
            //Word Count
            smb_response->pblock.word_count[0] = 0x00;

            //Words
            smb_response->pblock.words = calloc(1, 9);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[OPEN_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //FID
            *(smb_response->pblock.words) = fd;

            //FileAttrs
            *(smb_response->pblock.words + 1) = 0x00;

            //FileSize
            memcpy(smb_response->pblock.words +  2, &attrib.st_size, 4); 

            //AccessMode
            memcpy(smb_response->pblock.words +  6, &access_mode, 2); 
            break; 
    }


    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;

    return smb_response;
}

SMB_Struct *CREATE_RESP(int fd, int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[CREATE_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_OBJECT_NAME_NOT_FOUND;
            fprintf(stderr,"[CREATE_RESP] The name filed was not found\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 

            break;

        case 1:  
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr,"[CREATE_RESP] Path Syntax Invalid\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 

            break;

        case 2:  
            smb_response->header.error_status[0] = STATUS_INSUFF_SERVER_RESOURCES;
            fprintf(stderr,"[CREATE_RESP] The server is out of resources\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 
            break;

        case 3:  
            smb_response->header.error_status[0] = STATUS_INVALID_SMB;
            fprintf(stderr,"[CREATE_RESP] Invalid SMB, not parameter bytes sent\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 
            break;

        case 4:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_UID;
            fprintf(stderr,"[CREATE_RESP] The UID supplied is not known to the session\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 
            break;

        case 5:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr,"[CREATE_RESP] Disk Error I/O\n");

            //Parameters Block
            smb_response->pblock.word_count[0] = 0x00;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = 0x00; 
            break;

        case 6:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[CREATE_RESP] SUCCESS\n");
            
            //Parameters Block
            smb_response->pblock.word_count[0] = 0x01;

            smb_response->pblock.words = calloc(1, *(smb_response->pblock.word_count) * 2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[CREATE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            *(smb_response->pblock.words) = fd; 
            break;
    }


    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *CLOSE_RESP(int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[CLOSE_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_FID;
            fprintf(stderr,"[CLOSE_RESP] The FID provided is invalid\n");

            break;

        case 1:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[CLOSE_RESP] SUCCESS\n");

            break;

    }

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}


SMB_Struct *DELETE_RESP(int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[DELETE_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr, "[DELETE_RESP] The file name was not found\n");

            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr, "[DELETE_RESP] Path Syntax Invalid\n");

            break;

        case 2:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr, "[DELETE_RESP] SUCCESS\n");

            break;

    }

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *RENAME_RESP(int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[RENAME_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:    
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr,"[RENAME_RESP] File not found\n");

            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_OBJECT_NAME_COLLISION; 
            fprintf(stderr,"[RENAME_RESP] The new file name already exists\n");

            break;

        case 2:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[RENAME_RESP] SUCCESS\n");

            break;
    }

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *QUERY_INFORMATION_RESP(struct stat attrib, int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[QUERY_INFORMATION_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    struct tm *dt = localtime(&attrib.st_mtime);

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x0a;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr, "[QUERY_INFORMATION_RESP] The file does not exist\n");

            smb_response->pblock.words = calloc(1,17);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[QUERY_INFORMATION_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;

            }
            
            //FileAttr
            *(smb_response->pblock.words) = 0x00; 
            *(smb_response->pblock.words + 1) = 0x00; 

            //LastWriteTime
            *(smb_response->pblock.words + 2) = 0x00; 
            *(smb_response->pblock.words + 3) = 0x00; 
            *(smb_response->pblock.words + 4) = 0x00; 
            *(smb_response->pblock.words + 5) = 0x00; 

            //Reserved
            *(smb_response->pblock.words + 6) = 0x00; 
            *(smb_response->pblock.words + 7) = 0x00; 
            *(smb_response->pblock.words + 8) = 0x00; 
            *(smb_response->pblock.words + 9) = 0x00; 
            *(smb_response->pblock.words + 10) = 0x00; 
            *(smb_response->pblock.words + 11) = 0x00; 
            *(smb_response->pblock.words + 12) = 0x00; 
            *(smb_response->pblock.words + 13) = 0x00; 
            *(smb_response->pblock.words + 14) = 0x00; 
            *(smb_response->pblock.words + 15) = 0x00; 

            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_NOT_FOUND;
            fprintf(stderr, "[QUERY_INFORMATION_RESP] The file path syntax is invalid \n");

            smb_response->pblock.words = calloc(1,17);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[QUERY_INFORMATION_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            //FileAttr
            *(smb_response->pblock.words) = 0x00; 
            *(smb_response->pblock.words + 1) = 0x00; 

            //LastWriteTime
            *(smb_response->pblock.words + 2) = 0x00; 
            *(smb_response->pblock.words + 3) = 0x00; 
            *(smb_response->pblock.words + 4) = 0x00; 
            *(smb_response->pblock.words + 5) = 0x00; 

            //Reserved
            *(smb_response->pblock.words + 6) = 0x00; 
            *(smb_response->pblock.words + 7) = 0x00; 
            *(smb_response->pblock.words + 8) = 0x00; 
            *(smb_response->pblock.words + 9) = 0x00; 
            *(smb_response->pblock.words + 10) = 0x00; 
            *(smb_response->pblock.words + 11) = 0x00; 
            *(smb_response->pblock.words + 12) = 0x00; 
            *(smb_response->pblock.words + 13) = 0x00; 
            *(smb_response->pblock.words + 14) = 0x00; 
            *(smb_response->pblock.words + 15) = 0x00; 

            break;

        case 2:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr, "[QUERY_INFORMATION_RESP] Disk I/O Error\n");

            smb_response->pblock.words = calloc(1,17);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[QUERY_INFORMATION_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;


                return NULL;
            }
            
            //FileAttr
            *(smb_response->pblock.words) = 0x00; 
            *(smb_response->pblock.words + 1) = 0x00; 

            //LastWriteTime
            *(smb_response->pblock.words + 2) = 0x00; 
            *(smb_response->pblock.words + 3) = 0x00; 
            *(smb_response->pblock.words + 4) = 0x00; 
            *(smb_response->pblock.words + 5) = 0x00; 

            //Reserved
            *(smb_response->pblock.words + 6) = 0x00; 
            *(smb_response->pblock.words + 7) = 0x00; 
            *(smb_response->pblock.words + 8) = 0x00; 
            *(smb_response->pblock.words + 9) = 0x00; 
            *(smb_response->pblock.words + 10) = 0x00; 
            *(smb_response->pblock.words + 11) = 0x00; 
            *(smb_response->pblock.words + 12) = 0x00; 
            *(smb_response->pblock.words + 13) = 0x00; 
            *(smb_response->pblock.words + 14) = 0x00; 
            *(smb_response->pblock.words + 15) = 0x00; 

            break;

        case 3:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr, "[QUERY_INFORMATION_RESP] SUCCESS\n");

            smb_response->pblock.words = calloc(1,17);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[QUERY_INFORMATION_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }
            
            //FileAttr
            *(smb_response->pblock.words) = 0x00; 
            *(smb_response->pblock.words + 1) = 0x00; 

            printf("%d %d %d %d\n" , dt->tm_mday, dt->tm_sec, dt->tm_min, dt->tm_hour);
            //LastWriteTime
            *(smb_response->pblock.words + 2) = dt->tm_mday; 
            *(smb_response->pblock.words + 3) = dt->tm_sec; 
            *(smb_response->pblock.words + 4) = dt->tm_min; 
            *(smb_response->pblock.words + 5) = dt->tm_hour; 

            //Reserved
            *(smb_response->pblock.words + 6) = 0x00; 
            *(smb_response->pblock.words + 7) = 0x00; 
            *(smb_response->pblock.words + 8) = 0x00; 
            *(smb_response->pblock.words + 9) = 0x00; 
            *(smb_response->pblock.words + 10) = 0x00; 
            *(smb_response->pblock.words + 11) = 0x00; 
            *(smb_response->pblock.words + 12) = 0x00; 
            *(smb_response->pblock.words + 13) = 0x00; 
            *(smb_response->pblock.words + 14) = 0x00; 
            *(smb_response->pblock.words + 15) = 0x00; 

            break;
    }


    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *SET_INFORMATION_RESP(int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[SET_INFORMATION_RESP] Smb Response Struct Allocation failed\n");

        free(smb_response);
        smb_response = NULL;

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_NO_SUCH_FILE;
            fprintf(stderr,"[SET_INFORMATION_RESP] The file was not found\n");

            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_INVALID_PARAMETER;
            fprintf(stderr,"[SET_INFORMATION_RESP] One of the FileAttributes was invalid\n");

            break;

        case 2:
            smb_response->header.error_status[0] = STATUS_DATA_ERROR;
            fprintf(stderr,"[SET_INFORMATION_RESP] Disk I/O error\n");

            break;

        case 3:
            smb_response->header.error_status[0] = STATUS_NOT_IMPLEMENTED;
            fprintf(stderr,"[SET_INFORMATION_RESP] NOT IMPLEMENTED\n");

            break;

        case 4:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[SET_INFORMATION_RESP] SUCCESS\n");

            break;
    }
    //Parameters Block
    smb_response->pblock.word_count[0] = 0x00;
    smb_response->pblock.words = NULL;

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *READ_RESP(int error_code, int bytes_read, char *bytes){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[READ_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x05;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_FID;
            fprintf(stderr,"[READ_RESP] Attempt to read from a FID that the server does not have open\n");
            
            smb_response->pblock.words = calloc(1, 11);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[READ_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //CountOfBytes Read
            memset(smb_response->pblock.words, 0, 2);
            

            //Reserved
            memset(smb_response->pblock.words + 2, 0, 8);
            
            //Data Block 
            smb_response->dblock.byte_count[0] = 0x0003 + bytes_read;

            smb_response->dblock.bytes = calloc(1, 4 + bytes_read);

            if(smb_response->dblock.bytes == NULL){

                fprintf(stderr,"[READ_RESP] The allocation of words failed\n");

                free(smb_response->pblock.words);
                smb_response->pblock.words = NULL;

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            memset(smb_response->dblock.bytes, 0x01, 1);

            memcpy(smb_response->dblock.bytes + 1, &bytes_read, 2);

            break;

        case 1:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[READ_RESP] SUCCESS\n");

            smb_response->pblock.words = calloc(1, 11);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[READ_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            //CountOfBytes Read
            memcpy(smb_response->pblock.words, &bytes_read, 2);

            //Reserved
            memset(smb_response->pblock.words + 2, 0, 8);
            
            //Data Block 
            smb_response->dblock.byte_count[0] = 0x0003 + bytes_read;

            smb_response->dblock.bytes = calloc(1, 4 + bytes_read);

            if(smb_response->dblock.bytes == NULL){

                fprintf(stderr,"[READ_RESP] The allocation of words failed\n");

                free(smb_response->pblock.words);
                smb_response->pblock.words = NULL;

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            memset(smb_response->dblock.bytes, 0x01, 1);

            memcpy(smb_response->dblock.bytes + 1, &bytes_read, 2);

            memcpy(smb_response->dblock.bytes + 3, bytes, bytes_read);

            break;

    }



    return smb_response;
}

SMB_Struct *WRITE_RESP(int error_code, unsigned short bytes_written){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));
    
    if(smb_response == NULL){

        fprintf(stderr,"[WRITE_RESP] Smb Struct Response Allocation failed\n"); 

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    smb_response->pblock.word_count[0] = 0x01;

    switch(error_code){

        case 0:
            smb_response->header.error_status[0] = STATUS_SMB_BAD_FID;
            fprintf(stderr,"[WRITE_RESP] Invalid FID, or FID mapped to a valid server FID but it was not acceptable to the operating system\n");

            //Parameters Block
            smb_response->pblock.words = calloc(1,3);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[WRITE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

            }

            memset(smb_response->pblock.words, 0x00, 2);

            break;
            
        case 1:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[WRITE_RESP] SUCCESS\n");
            
            //Parameters Block
            smb_response->pblock.words = calloc(1,3);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr,"[WRITE_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

            }

            memcpy(smb_response->pblock.words, &bytes_written, 2);

            break;
    
}

    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *CREATE_NEW_RESP(int error_code, int fid){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[CREATE_NEW_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    //Req_header  
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0000;

    smb_response->header.MID[0] = 0x0000;

    smb_response->pblock.word_count[0] = 0x01;

    switch(error_code){
        case 0:
            smb_response->header.error_status[0] = STATUS_OBJECT_PATH_SYNTAX_BAD;
            fprintf(stderr,"[CREATE_NEW_RESP] File Path Syntax Invalid\n");

            //Parameters Block
            smb_response->pblock.words = calloc(1,2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr, "[CREATE_NEW_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            memset(smb_response->pblock.words, 0x00, 2);
            break;

        case 1:
            smb_response->header.error_status[0] = STATUS_OBJECT_NAME_COLLISION;
            fprintf(stderr,"[CREATE_NEW_RESP] A file with this name already exists\n");

            //Parameters Block
            smb_response->pblock.words = calloc(1,2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr ,"[CREATE_NEW_RESP] The allocation of words failed\n");
                
                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            memset(smb_response->pblock.words, 0x00, 2);
            break;
            
        case 2:
            smb_response->header.error_status[0] = SUCCESS;
            fprintf(stderr,"[CREATE_NEW_RESP] SUCCESS\n");

            //Parameters Block
            smb_response->pblock.words = calloc(1,2);

            if(smb_response->pblock.words == NULL){

                fprintf(stderr, "[CREATE_NEW_RESP] The allocation of words failed\n");

                free(smb_response);
                smb_response = NULL;

                return NULL;
            }

            memset(smb_response->pblock.words, fid, 1);
            break;
}


    //Data Block 
    smb_response->dblock.byte_count[0] = 0x0000;
    smb_response->dblock.bytes = NULL;


    return smb_response;
}

SMB_Struct *NEGOTIATE_RESP(SMB_Struct *rq, int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[NEGOTIATE_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    switch(error_code){

    }

    //Parameters Block
    smb_response->pblock.word_count[0] = 0x11;

    smb_response->pblock.words = calloc(1,34);

    if(smb_response->pblock.words == NULL){

        fprintf(stderr, "[NEGOTIATE_RESP] The allocation of words failed\n");

        free(smb_response);
        smb_response = NULL;

        return NULL;
    }


    //Check if NT Lan Man is the dialect sent
    if(strncmp(rq->dblock.bytes + 1 , "NT LM 0.12", 10) == 0){

        smb_response->header.protocol[0] = 0x00;
        smb_response->header.protocol[1] = 0x00;
        smb_response->header.protocol[2] = 0x00;
        smb_response->header.protocol[3] = 0x00;

        smb_response->header.command[0] = 0x00;

        smb_response->header.error_status[0] = 0x00000000;

        smb_response->header.flags1[0] = 0x00;

        smb_response->header.flags2[0] = 0x0000;

        smb_response->header.PIDhigh[0] = 0x0000;

        smb_response->header.reserved[0] = 0x0000;

        smb_response->header.TID[0] = 0x0000;

        smb_response->header.PIDlow[0] = 0x0000;

        smb_response->header.UID[0] = 0x0000;

        smb_response->header.MID[0] = 0x0000;

        //Server assumes only one dialect is sent 2 bytes : 0x0000
        *smb_response->pblock.words = 0x00;
        *(smb_response->pblock.words + 1) = 0x00;

        //Security Mode 8-bit mask 1 byte : 0x01
        *(smb_response->pblock.words + 2) = 0x1;

        //MaxMpxCount 2 bytes: 0x0001
        *(smb_response->pblock.words + 3) = 0x00;
        *(smb_response->pblock.words + 4) = 0x01;


        //MaxNumberVcs 2 bytes 0x0000
        *(smb_response->pblock.words + 5) = 0x00;
        *(smb_response->pblock.words + 6) = 0x00;


        //MaxBufferSize 4 bytes MAX_BUFFER_SIZE
        *(smb_response->pblock.words + 7) = 0x00;
        *(smb_response->pblock.words + 8) = 0x00;
        *(smb_response->pblock.words + 9) = 0x11;
        *(smb_response->pblock.words + 10) = 0x04;


        //MaxRawSize 4 bytes 0x00000000
        *(smb_response->pblock.words + 11) = 0x00;
        *(smb_response->pblock.words + 12) = 0x00;
        *(smb_response->pblock.words + 13) = 0x00;
        *(smb_response->pblock.words + 14) = 0x00;


        //SessionKey 4 bytes
        *(smb_response->pblock.words + 15) = 0x00;
        *(smb_response->pblock.words + 16) = 0x00;
        *(smb_response->pblock.words + 17) = 0x00;
        *(smb_response->pblock.words + 18) = 0x00;


        //Capabilities 4 bytes 0x00000048
        *(smb_response->pblock.words + 19) = 0x00;
        *(smb_response->pblock.words + 20) = 0x00;
        *(smb_response->pblock.words + 21) = 0x00;
        *(smb_response->pblock.words + 22) = 0x48;


        //SystemTime 8 Bytes 100 Nano Seconds since Jan 1601 0xXXXXXXXXXXXXXXXX
        long double nano = system_time();
        char nano_string[17];

        snprintf(nano_string, 17 , "%.0Lf", nano);
        nano_string[16] = '\0';


        int x = 0, i = 0;
        for(i = 0, x = 23; i < 15; i += 2, ++x){
            *(smb_response->pblock.words + x) = (nano_string[i] - 48) *  10 + (nano_string[i + 1] - 48);
            printf("smb_response->pblock.words: %x\n" , *(smb_response->pblock.words + x));
        }

        //SystemTimeZone 2 byte Signed Int, hour diff from UTC time
        *(smb_response->pblock.words + 31) = 0x01;
        *(smb_response->pblock.words + 32) = 0x00;


        //ChallengeLength 1 byte: must be 0x00 not supporting User Level Access
        *(smb_response->pblock.words + 33) = 0x00;


        //Data_Block
        smb_response->dblock.byte_count[0] = 0x1000;

        smb_response->dblock.bytes = calloc(1,17);

        if(smb_response->dblock.bytes == NULL){

            fprintf(stderr, "[NEGOTIATE_RESP] Failed to allocate bytes\n");
                
            free(smb_response->pblock.words);
            smb_response->pblock.words = NULL;

            free(smb_response);
            smb_response = NULL;

            return NULL;

        }

        strncpy(smb_response->dblock.bytes, WORKGROUP_NAME, 16);

    } else {

        fprintf(stderr, "NT LAN is the only dialect allowed\n");

        free(smb_response->pblock.words);
        smb_response->pblock.words = NULL;

        free(smb_response);
        smb_response = NULL;

        return NULL;
    }

    return smb_response;
}

SMB_Struct *SETUP_ANDX_RESP(SMB_Struct *rq, int error_code){

    SMB_Struct *smb_response = malloc(sizeof(SMB_Struct));

    if(smb_response == NULL){

        fprintf(stderr,"[SETUP_ANDX_RESP] Smb Response Struct Allocation failed\n");

        return NULL;
    }

    switch(error_code){

    }

    //Header
    smb_response->header.protocol[0] = 0x00;
    smb_response->header.protocol[1] = 0x00;
    smb_response->header.protocol[2] = 0x00;
    smb_response->header.protocol[3] = 0x00;

    smb_response->header.command[0] = 0x00;

    smb_response->header.error_status[0] = 0x00000000;

    smb_response->header.flags1[0] = 0x00;

    smb_response->header.flags2[0] = 0x0000;

    smb_response->header.PIDhigh[0] = 0x0000;

    smb_response->header.reserved[0] = 0x0000;

    smb_response->header.TID[0] = 0x0000;

    smb_response->header.PIDlow[0] = 0x0000;

    smb_response->header.UID[0] = 0x0001;

    smb_response->header.MID[0] = 0x0000;

    //Words
    //WordCount
    smb_response->pblock.word_count[0] = 0x03;

    //Words
    smb_response->pblock.words = calloc(1,7);

    if(smb_response->pblock.words == NULL){

        fprintf(stderr, "[SETUP_ANDX_RESP] Words allocation failed\n"); 

        free(smb_response);
        smb_response = NULL;

        return NULL;
    }

    //AndXCommand 1
    *(smb_response->pblock.words) = 0xFF;

    //AndXReserved 1
    *(smb_response->pblock.words + 1) = 0x00;

    //AndXOffset 2
    *(smb_response->pblock.words + 2) = 0x00;
    *(smb_response->pblock.words + 3) = 0x00;

    //Action 2
    *(smb_response->pblock.words + 4) = 0x00;
    *(smb_response->pblock.words + 5) = 0x00;


    //DataBlock
    //ByteCount
    smb_response->dblock.byte_count[0] = 38;

    smb_response->dblock.bytes = calloc(1, *(smb_response->dblock.byte_count) + 1);

    if(smb_response->dblock.bytes== NULL){

        fprintf(stderr, "[SETUP_ANDX_RESP] Words allocation failed\n"); 

        free(smb_response->pblock.words);
        smb_response->pblock.words = NULL;

        free(smb_response);
        smb_response = NULL;

        return NULL;
    }

    //Native OS
    memcpy(smb_response->dblock.bytes , NATIVE_OS , 12);

    //NativeLanMan
    memcpy(smb_response->dblock.bytes + 12, NATIVE_LAN_MAN, 10);

    //Primary Domain
    memcpy(smb_response->dblock.bytes + 22, WORKGROUP_NAME, 16);

    return smb_response;
}
