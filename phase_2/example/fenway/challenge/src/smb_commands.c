#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>

#include "smb_commands.h"
#include "smb_response.h"
#include "pgm.h"
#include "ppm.h"

//UID, TID, Path Relative to share
//Check for create permissions within parent directory
//Who has access to created policy, local policy
int SMB_COM_CREATE_DIR(int fd, char *dir_name){
    fprintf(stderr,"[SMB_COM_CREATE_DIR]\n\n");

    SMB_Struct *resp = NULL;
    int access_check = -1, mkdir_check = -1;

    if(dir_name == NULL){
        fprintf(stderr, "[SMB_COM_CREATE_DIR] No directory name given\n");
        resp = CREATE_DIR_RESP(0);

    } else if((access_check = access(dir_name, F_OK)) == 0){
        fprintf(stderr, "[SMB_COM_CREATE_DIR] The directory already exists\n");
        resp = CREATE_DIR_RESP(3);

    } else if((access_check != 0) && ((mkdir_check = mkdir(dir_name, 0700)) == -1)){
        fprintf(stderr, "[SMB_COM_CREATE_DIR] Mkdir failure\n");
        resp = CREATE_DIR_RESP(0);

    } else {

        fprintf(stderr, "[SMB_COM_CREATE_DIR] Directory created\n");
        resp = CREATE_DIR_RESP(9);
    }

    int sender = -2;

    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp);
    resp = NULL;

    return 1;
}


int SMB_COM_DELETE_DIR(int fd, char *dir_name){
    fprintf(stderr,"[SMB_COM_DELETE_DIR]\n\n");
    SMB_Struct *resp = NULL;

    int access_check = -1, remove_check = -1;

    if(dir_name == NULL){
        fprintf(stderr, "[SMB_COM_DELETE_DIR] No directory name given\n");
        resp = DELETE_DIR_RESP(1); 

    } else if((access_check = access(dir_name, F_OK)) != 0){
        fprintf(stderr, "[SMB_COM_DELETE_DIR] The directory does not exist\n");
        resp = DELETE_DIR_RESP(0);

    } else if((remove_check = remove(dir_name)) != 0){
        fprintf(stderr, "[SMB_COM_DELETE_DIR] The directory failed to be removed\n");
        resp = DELETE_DIR_RESP(11);

    } else {
        fprintf(stderr, "[SMB_COM_DELETE_DIR] The directory was removed\n");
        resp = DELETE_DIR_RESP(12);
    }

    int sender = -2;

    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        perror("Error printed by perror");

        return 0;
    }

    //Word Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp);
    resp = NULL;

    return 1;
}


int SMB_COM_OPEN(int fd, char *path, uint8_t access_mode){
    fprintf(stderr,"[SMB_COM_OPEN]\n\n");

    SMB_Struct *resp = NULL;
    int access_check = -1, open_check = -2;

    if(path == NULL){

        fprintf(stderr, "[SMB_COM_OPEN] No path name given\n");
        resp = OPEN_RESP(1, -1, NULL, 0);

    } else if((access_check = access(path, F_OK)) != 0){

        fprintf(stderr, "[SMB_COM_OPEN] The file does not exist\n");
        resp = OPEN_RESP(0, -1, NULL, 0);

    } else {

        if(access_mode == 0){
            if((open_check = open(path,O_RDONLY)) == -1){
                fprintf(stderr, "[SMB_COM_OPEN] There was a problem opening for reading\n");   
                resp = OPEN_RESP(5, -1, NULL, 0);

            } else {
                resp = OPEN_RESP(6, open_check, path, access_mode);
                fprintf(stderr, "[SMB_COM_OPEN] The file was successfully opened\n");
            }

        } else if(access_mode == 1){
            if((open_check = open(path,O_WRONLY | O_TRUNC)) == -1){
                fprintf(stderr, "[SMB_COM_OPEN] There was a problem opening for writing\n");   
                resp = OPEN_RESP(5, -1, NULL, 0);

            } else {
                resp = OPEN_RESP(6, open_check, path, access_mode);
                fprintf(stderr, "[SMB_COM_OPEN] The file was successfully opened\n");
            }

        } else if(access_mode == 2){
            if((open_check = open(path,O_RDWR)) == -1){
                fprintf(stderr, "[SMB_COM_OPEN] There was a problem opening for reading and writing\n");   
                resp = OPEN_RESP(5, -1, NULL, 0);

            } else {
                resp = OPEN_RESP(6, open_check, path, access_mode);
                fprintf(stderr, "[SMB_COM_OPEN] The file was successfully opened\n");
            }

        } else {
            fprintf(stderr, "[SMB_COM_OPEN] Something was wrong with the access mode\n");
            resp = OPEN_RESP(5,-1, NULL, 0);
        }

    }

    //Send Response
    int sender = -2;

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 8, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){

        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp);
    resp = NULL;

    return 1;
}

//Create and open a new file, truncate an
//an existing file to zero length
PGM *SMB_COM_CREATE(int fd, char *path, uint64_t version){
    fprintf(stderr,"[SMB_COM_CREATE]\n\n");

    SMB_Struct *resp = NULL;
    PGM  *pgm;
    int file_d = 0, sender = -2;

    if(path == NULL){
        fprintf(stderr, "[SMB_COM_CREATE] No path given\n");
        resp = CREATE_RESP(file_d, 1);

    }  else if((pgm = create_pgm(path, version)) == NULL){
        fprintf(stderr, "[SMB_COM_CREATE] There was a problem creating the file\n");
        resp = CREATE_RESP(file_d, 5);

    } else if((file_d = open(path, O_RDWR)) == -1){
            fprintf(stderr, "[SMB_COM_CREATE] There was a problem opening the file\n");
            resp = CREATE_RESP(file_d, 5);
    
    } else {

        resp = CREATE_RESP(file_d, 6);
    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){
        fprintf(stderr, "Failed to send header\n");
        return NULL;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return NULL;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return NULL;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return NULL;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp);
    resp = NULL;

    return pgm;
}


//Given a FID close that object
int SMB_COM_CLOSE(int fd, int fd_to_close){
    fprintf(stderr,"[SMB_COM_CLOSE]\n\n");
    SMB_Struct *resp = NULL;


    int close_check = -1, sender = -2;

    if((close_check = close(fd_to_close)) == -1 || fd == fd_to_close || fd_to_close < 3){
        resp = CLOSE_RESP(0);
        fprintf(stderr, "[SMB_COM_CLOSE] Failed to close the file\n");
    } else {
        fprintf(stderr, "[SMB_COM_CLOSE] SUCCESS\n");
        resp = CLOSE_RESP(1);
    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp);
    resp = NULL;

    return 1;
}


int SMB_COM_DELETE(int fd, char *dir_name){

    fprintf(stderr, "[SMB_COM_DELETE]\n\n");  

    SMB_Struct *resp = NULL;
    int delete_check = -1, sender = -2;

    if(dir_name == NULL){
        fprintf(stderr,"[SMB_COM_DELETE] Directory name provided was NULL\n");
        resp = DELETE_RESP(1);

    } else if((delete_check = remove(dir_name)) != 0){
        fprintf(stderr, "[SMB_COM_DELETE] There was an error deleting the file\n");
        resp = DELETE_RESP(0);

    } else {
        fprintf(stderr, "[SMB_COM_DELETE] SUCCESS\n");
        resp = DELETE_RESP(2);
    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }


    free(resp);
    resp = NULL;

    return 1;
}


//Changes the name of one or more files or directories
//Has wildcard support
int SMB_COM_RENAME(int fd, char *dir_name, char *change_name){
    fprintf(stderr, "[SMB_COM_RENAME]\n\n");  

    SMB_Struct *resp = NULL;
    int rename_check = -1, sender = -2;

    if(dir_name == NULL){

        fprintf(stderr, "[SMB_COM_RENAME] Old Dir was NULL\n");

        resp = RENAME_RESP(0);
    }

    if(change_name == NULL){

        fprintf(stderr, "[SMB_COM_RENAME] Change name was NULL\n");

        resp = RENAME_RESP(0);
    }

    if((rename_check = rename(dir_name, change_name)) == -1){
        fprintf(stderr, "[SMB_COM_RENAME] There was an error while renaming the file\n");
        resp = RENAME_RESP(0);

    } else {
        fprintf(stderr, "[SMB_COM_RENAME] SUCCESS\n");
        resp = RENAME_RESP(2);

    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }


    free(resp);
    resp = NULL;

    return 1;
}


//Returns information about file in question
int SMB_COM_QUERY_INFORMATION(int fd, char *path){
    fprintf(stderr, "[SMB_COM_QUERY_INFORMATION]\n\n");  

    SMB_Struct *resp = NULL;
    struct stat attrib;
    int sender = 2;

    if(stat(path, &attrib) == -1){
        fprintf(stderr, "[SMB_COM_QUERY_INFORMATION] Stat failed\n"); 
        resp = QUERY_INFORMATION_RESP(attrib, 0);

    } else {
        fprintf(stderr, "[SMB_COM_QUERY_INFORMATION] SUCCESS\n"); 
        resp = QUERY_INFORMATION_RESP(attrib, 3);
    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 16, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp);
    resp = NULL;

    return 1;
}

int SMB_COM_SET_INFORMATION(int fd, char *path){
    fprintf(stderr,"[SMB_COM_SET_INFORMATION]\n\n");

    SMB_Struct *resp = NULL;
    int sender = -2;

    resp = SET_INFORMATION_RESP(3);

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    return 1;
}

//Used to read bytes from a regular file
//EOF condition is reached by server sending back less bytes than the client 
int SMB_COM_READ(int fd, unsigned int fid){
    fprintf(stderr, "[SMB_COM_READ]\n\n");  

    SMB_Struct *resp = NULL;
    int sender = -2, read_check = 0;
    char *bytes = NULL;

    if(fid < 3){
        resp = READ_RESP(0, 0, bytes);
    } else {

        unsigned short size = lseek(fid, 0 , SEEK_END);
        lseek(fid, 0, SEEK_SET);
        bytes = calloc(1, size + 1);

        if(bytes == NULL){
            fprintf(stderr, "[SMB_COM_READ] The dynamic allocation of bytes failed\n");
            return 0; 
        }


        if((read_check = read(fid, bytes, size)) == -1){
            fprintf(stderr, "[SMB_COM_READ] There was a problem with read\n");
            resp = READ_RESP(0, read_check, bytes);
        } else {
            fprintf(stderr, "[SMB_COM_READ] SUCCESS\n");

            printf("File contents: %s\n" , bytes);
            printf("Read: %d\n" , read_check);

            resp = READ_RESP(1, read_check, bytes);
        }
    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 10, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    if(read_check == 0){
        if((sender = send(fd, resp->pblock.words, 3, MSG_NOSIGNAL)) == -1){
            fprintf(stderr,"Failed to send words\n"); 
            return 0;
        }
    } else {
        if((sender = send(fd, resp->dblock.bytes, 3 + read_check, MSG_NOSIGNAL)) == -1){
            fprintf(stderr,"Failed to send words\n"); 
            return 0;
        }
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp->dblock.bytes);
    resp->dblock.bytes = NULL;

    free(resp);
    resp = NULL;

    return 1;
}

//Used to write bytes to a regular file
//TID and FID Required
//Bytes written beyond the scope of the current file
//must write that many zeros
int SMB_COM_WRITE(int fd, unsigned int fid, char *bytes, unsigned int bytes_to_write){

    fprintf(stderr, "[SMB_COM_WRITE]\n\n");  
    SMB_Struct *resp = NULL;
    int sender = -2, write_check= -2;


    if(fid < 3){
        resp = WRITE_RESP(0, 0);

    } else if((write_check = write(fid, bytes, bytes_to_write))== -1){
            fprintf(stderr, "[SMB_COM_WRITE] There was a problem with write\n");
            perror("Error: ");
            resp = WRITE_RESP(0, write_check);

    } else {
            fprintf(stderr, "[SMB_COM_WRITE] SUCCESS\n");
            resp = WRITE_RESP(1, write_check);
    }
    

    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return 0;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return 0;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp);
    resp = NULL;

    return 1;
}

//Create new file that does not exist, cannot overwrite or trunc other files
PPM *SMB_COM_CREATE_NEW(int fd, char *path, uint64_t version){

    fprintf(stderr, "[SMB_COM_CREATE_NEW]\n\n");  

    SMB_Struct *resp = NULL;
    PPM *ppm;
    int open_check = -2, sender = -2;

    if((ppm = create_ppm(path, version)) == NULL){
        fprintf(stderr, "[SMB_COM_CREATE] There was a problem creating the file\n");
        resp = CREATE_NEW_RESP(0,0);

    } else if((open_check = open(path, O_RDWR)) == -1){
        fprintf(stderr, "[SMB_COM_CREATE_NEW] Failed to create the file\n");
        resp = CREATE_NEW_RESP(0,0);

    } else {
        fprintf(stderr, "[SMB_COM_CREATE_NEW] SUCCESS\n");
        resp = CREATE_NEW_RESP(2, open_check);

    }

    //Header
    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        fprintf(stderr, "Failed to send header\n");
        return NULL;
    }

    //Word_Count
    if((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return NULL;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return NULL;
    }

    //Send Byte Count
    if((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return NULL;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp);
    resp = NULL;

    return ppm;
}

//Beginning communication between server and client
//Server Sends back resposne from client intitation
int SMB_COM_NEGOTIATE(int fd, SMB_Struct *rq, int error_code){

    SMB_Struct *resp = NEGOTIATE_RESP(rq, 0);

    int sender = -2;

    if((sender = send(fd, &resp->header, sizeof(Req_Header), MSG_NOSIGNAL)) == -1){

        printf("Failed to send response\n");

        return 0;
    }

    //Send Param Block
    //Word Count
    if((sender = send(fd, resp->pblock.word_count, sizeof(uint8_t), MSG_NOSIGNAL)) == -1){

        printf("Failed to send word_count\n");

        return 0;
    }

    //Words
    if((sender = send(fd, resp->pblock.words, 34, MSG_NOSIGNAL)) == -1){

        printf("Failed to send response\n");

        return 0;
    }

    //Send Data Block
    //Byte Count
    if((sender = send(fd, resp->dblock.byte_count, sizeof(uint16_t), MSG_NOSIGNAL)) == -1){

        printf("Failed to send response\n");

        return 0;
    }

    if((sender = send(fd, resp->dblock.bytes, strlen(resp->dblock.bytes), MSG_NOSIGNAL)) == -1){

        printf("Failed to send response\n");

        return 0;
    }

    free(resp->pblock.words);
    resp->pblock.words = NULL;

    free(resp->dblock.bytes);
    resp->dblock.bytes = NULL;

    free(resp);
    resp = NULL;

    return 1;
}

//ANDX for TreeConnect, which enables a drive connection
int SMB_COM_SETUP_ANDX(int fd, SMB_Struct *rq, int error_code){

    fprintf(stderr, "[SMB_COM_SETUP_ANDX]\n\n");
    SMB_Struct *resp = SETUP_ANDX_RESP(rq, 0);

    int sender = -2;


    while((sender = send(fd, &resp->header, sizeof(Req_Header),MSG_NOSIGNAL)) < sizeof(Req_Header));

    if(sender == -1){
        fprintf(stderr,"Failed to send request header\n"); 
        perror("Error: ");
        return 0;
    }

    sender = 0;

    //Send Param Block
    //Word Count
    while((sender = send(fd, resp->pblock.word_count, 1, MSG_NOSIGNAL)) < 1);

    if(sender == -1){
        fprintf(stderr,"Failed to send word count\n"); 
        return 0;
    }


    sender = 0;

    //Words
    while((sender = send(fd, resp->pblock.words, 6, MSG_NOSIGNAL)) < 6);

    if(sender == -1){
        fprintf(stderr,"Failed to send words\n"); 
        return 0;
    }


    sender = 0;

    //Send Data Block
    //Byte Count
    while((sender = send(fd, resp->dblock.byte_count, 2, MSG_NOSIGNAL)) < 2);

    if(sender == -1){
        fprintf(stderr,"Failed to send byte count\n"); 
        return 0;
    }


    sender = 0;

    while((sender = send(fd, resp->dblock.bytes, 38, MSG_NOSIGNAL)) < 38);

    if(sender == -1){
        fprintf(stderr,"Failed to send bytes\n"); 
        return 0;
    }


    sender = 0;

    free(resp);
    resp = NULL;

    return 1;
}
