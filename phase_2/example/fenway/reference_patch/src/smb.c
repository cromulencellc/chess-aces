#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "smb.h"
#include "smb_commands.h"
#include "smb_response.h"
#include "ppm.h"
#include "pgm.h"

#define TIMEOUT 500
#define MAX_BYTE_SIZE 0x4b320 
#define MAX_WORD_SIZE 500

#define HIGH_COMMAND 0xe
#define LOW_COMMAND 0x0
#define MAX_PID 32768
#define MAX_TID 32
#define MAX_UID 32
#define MAX_MID 32

PGM *pgm;
PPM *ppm;
uint64_t version;

SMB_Struct *SMB_parse(int fd){

    SMB_Struct *smb = malloc(sizeof(SMB_Struct));

    if(smb == NULL){
        fprintf(stderr, "\n[SMB_PARSER] Failed to allocate memory for SMB Struct\n");

        return NULL;
    }

    PGM *pgm;
    PPM *ppm;
    uint64_t version;

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    int poll_check = poll(fds, 1, TIMEOUT);

    if(poll_check < 0){

        fprintf(stderr, "\n[SMB_PARSER] Error with Protocol Poll\n");
        perror("Error Message: ");


        free(smb);
        smb = NULL;

        return NULL;

    } else if(poll_check == 0){

        fprintf(stderr, "\n[SMB_PARSER] Protocol Poll Timeout\n");

        free(smb);
        smb = NULL;

        return NULL;

    }

    //Making sure all necessary fields for the command are present
    //Request Header

    int reader;

    //READ PROTOCOL {xff , 'S' , 'M', 'B'}
    if(fds[0].revents & POLLIN){

        if((reader = read(fd, smb->header.protocol, 4)) == -1){
            fprintf(stderr,"\n[SMB_PARSER] Reading Protocol Error\n");

            free(smb);
            smb = NULL;

            return NULL;
        }

        if(smb->header.protocol[0] == 0xff & smb->header.protocol[1] == 'S' & smb->header.protocol[2] == 'M' & 
                smb->header.protocol[3] == 'B'){

            fprintf(stderr, "\n[SMB_PARSER] Protcol Check Passed\n");

        } else {

            fprintf(stderr, "\n[SMB_PARSER] Protcol Check Failed\n");  

            free(smb);
            smb = NULL;

            return NULL;
        }

    } else {

        fprintf(stderr, "\n[SMB_PARSER] Protcol Revents Poll Error\n");  

        free(smb);
        smb = NULL;

        return NULL;
    }

    poll_check = poll(fds, 1, TIMEOUT);

    if(poll_check < 0){

        fprintf(stderr, "\n[SMB_PARSER] Error in Command Poll\n");
        perror("Error Message: ");

        free(smb);
        smb = NULL;

        return NULL;

    } else if(poll_check == 0){

        fprintf(stderr, "\n[SMB_PARSER] Command Poll Timeout\n");

        free(smb);
        smb = NULL;

        return NULL;

    }

    //READ command
    if(fds[0].revents & POLLIN){

        reader = 0;

        if((reader = read(fd, smb->header.command, 1)) == -1){

            fprintf(stderr,"\n[SMB_PARSER] Reading Command Error\n");
            perror("Error Message: ");

            free(smb);
            smb = NULL;

            return NULL;
        }

        if(smb->header.command[0] >= LOW_COMMAND && smb->header.command[0] <= HIGH_COMMAND){

            fprintf(stderr, "\n[SMB_PARSER] Command Check Passed\n");

        } else {

            fprintf(stderr, "\n[SMB_PARSER] Command Check Failed\n");  

            free(smb);
            smb = NULL;

            return NULL;
        }

    } else {

        fprintf(stderr, "\n[SMB_PARSER] Command Revents Poll Error\n");  
        perror("Error Message: ");

        free(smb);
        smb = NULL;

        return NULL;
    }

    switch(*(smb->header.command)){
        case 0:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Passed\n\n");

            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1 ){
                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Failed\n");

                smb->dblock.byte_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Passed\n\n");

            }

            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){
                fprintf(stderr,"\n[SNB_PARSER] Failed to allocate space for Data Block bytes.\n\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Bytes Error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            char *buffer_format = calloc(1,3);
            if(buffer_format == NULL){
                fprintf(stderr,"\n[SNB_PARSER] Failed to allocate space for buffer_format.\n\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            char *dir_name = calloc(1, MAX_BYTE_SIZE + 1);
            if(dir_name == NULL){
                fprintf(stderr,"\n[SNB_PARSER] Failed to allocate space for directory name\n\n");

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(dir_name, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Dir_name: %s\n" , dir_name);

            print_req_header(smb->header);

            print_param_block(smb->pblock);

            print_data_block(smb->dblock);

            int checker = -1;
            if((checker = SMB_COM_CREATE_DIR(fd, dir_name)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return smb;
            }

            break;

        case 1:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Passed\n\n");

            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1 ){
                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Failed\n");

                smb->dblock.byte_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Passed\n\n");

            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Bytes Error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;


                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            dir_name = calloc(1, MAX_BYTE_SIZE + 1);

            if(dir_name == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(dir_name, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Dir_name: %s\n" , dir_name);


            if((checker = SMB_COM_DELETE_DIR(fd, dir_name)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;


                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(buffer_format);
                buffer_format = NULL;

                free(dir_name);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return smb;

            }

            break;

        case 2:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 2){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Passed\n\n");

            }
            //Words
            reader = 0;

            smb->pblock.words = calloc(1, *smb->pblock.word_count * 2);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 4)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Words Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Access Mode
            uint8_t access_mode; 
            memcpy(&access_mode, smb->pblock.words, 1);

            //Search Attributes
            char *search_attr = calloc(1, 3); 
            if(search_attr == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            memcpy(search_attr, smb->pblock.words + 2, 2);


            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1 ){
                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                search_attr = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Failed\n");

                smb->dblock.byte_count[0] = 0;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                search_attr = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Passed\n\n");

            }

            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                search_attr = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Bytes Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                search_attr = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                search_attr = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            char *path = calloc(1, MAX_BYTE_SIZE + 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                path = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(path, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Path: %s\n" , path);
            printf("Access Mode: %c\n" , (char)access_mode + 48);

            if((checker = SMB_COM_OPEN(fd, path, access_mode)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                path = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(buffer_format);
                path = NULL;

                free(path);
                path = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                path = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(buffer_format);
                path = NULL;

                free(path);
                path = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(search_attr);
                path = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(buffer_format);
                path = NULL;

                free(path);
                path = NULL;

                return smb;
            }

            break;

        case 3:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                return NULL;
            }

            if(smb->pblock.word_count[0] != 3){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "[SMB_PARSER] Word Count Check Passed\n\n");

            }

            reader = 0;
            smb->pblock.words = calloc(1, 7 + 1);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 7)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Words Read Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Failed\n");

                smb->dblock.byte_count[0] = 0;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr,"\n[SMB_PARSER] Byte Count Check Passed\n\n");

            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE+ 1);

            if(smb->dblock.bytes == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Read Bytes Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            memcpy(&version, smb->dblock.bytes + 2, 8);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE + 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(path, smb->dblock.bytes + 10, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Version: 0x%" PRIx64 "\n" , version);
            printf("Path: %s\n" , path);

            if((pgm = SMB_COM_CREATE(fd, path,version)) == NULL){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

            }

            break;

        case 4:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 1){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr,"\n[SMB_PARSER] Word Count Check Passed\n");

            }

            reader = 0;
            smb->pblock.words = calloc(1, 2 + 1);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Error\n");

                perror("ERROR: ");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            if(smb->dblock.byte_count[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            int file_d = *smb->pblock.words;

            if((checker = SMB_COM_CLOSE(fd, file_d)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                return smb;

            }

            break;

        case 5:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 1){

                fprintf(stderr, "\n[SMB_PARSER] Invalid Word_Count field for request\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }

            reader = 0;
            smb->pblock.words = calloc(1, 3);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Word read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE + 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(path, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Path: %s\n" , path);

            if((checker = SMB_COM_DELETE(fd, path)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return smb;

            }

            break;

        case 6:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Word_Count read error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 1){

                fprintf(stderr, "\n[SMB_PARSER] Invalid Word_Count field for request\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }

            reader = 0;
            smb->pblock.words = calloc(1, 3);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Word_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                smb->dblock.byte_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                smb->dblock.byte_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE + 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(path, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            char *buffer_format2 = calloc(1,3);

            if(buffer_format2 == NULL){

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(buffer_format2, smb->dblock.bytes + (3 + strlen(path)) , 2);

            char *path2 = calloc(1, MAX_BYTE_SIZE + 1);

            if(path2 == NULL){

                free(buffer_format2);
                buffer_format2 = NULL;

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(path2, smb->dblock.bytes + (5 + strlen(path)), MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Path: %s\n" , path);
            printf("Buffer format2: %s\n" , buffer_format2);
            printf("Path2: %s\n" , path2);

            if((checker = SMB_COM_RENAME(fd, path, path2)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(buffer_format2);
                buffer_format2 = NULL;

                free(path2);
                path2 = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(buffer_format2);
                buffer_format2 = NULL;

                free(path2);
                path2 = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(buffer_format);
                buffer_format = NULL;

                free(path);
                path = NULL;

                free(buffer_format2);
                buffer_format2 = NULL;

                free(path2);
                path2 = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                return smb;

            }

            break;

        case 7:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Invalid Word_Count field for request\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Byte Count Check Failed\n");

                smb->dblock.byte_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if(smb->dblock.bytes == NULL){

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Bytes Error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE + 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            strncpy(path, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            if((checker = SMB_COM_QUERY_INFORMATION(fd, path)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;


                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return smb;

            }

            break;

        case 8:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 8){

                fprintf(stderr, "\n[SMB_PARSER] Invalid Word_Count field for request\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->pblock.words = calloc(1, 17);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 16)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Word read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }


            unsigned int *file_attr = calloc(1, 3); 
            memcpy(file_attr, smb->pblock.words, 2);

            unsigned int *last_write_time = calloc(1,5);
            memcpy(last_write_time, smb->pblock.words + 2, 4);

            unsigned int *reserved = calloc(1,11);
            memcpy(reserved, smb->pblock.words + 6 , 10);

            printf("File Attr: %x\n", *file_attr);
            printf("Last Write Time: %x\n", *last_write_time);
            printf("Resereved: %x\n", *reserved);

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){
                if(reader == -1){
                    fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                    free(smb->pblock.words);
                    smb->pblock.words = NULL;

                    free(smb);
                    smb = NULL;

                    free(smb->dblock.bytes);
                    smb->dblock.bytes = NULL;

                    return NULL;

                }
            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);
            strncpy(buffer_format, smb->dblock.bytes, 2);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE + 1);
            strncpy(path, smb->dblock.bytes + 2, MAX_BYTE_SIZE);

            if((checker = SMB_COM_SET_INFORMATION(fd, path)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                return NULL;

            } else {

                return smb;

            }

            break;

        case 9:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }
            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){

                fprintf(stderr,"[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;

            }

            if(smb->pblock.word_count[0] != 5){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Passed");

            }

            reader = 0;
            smb->pblock.words = calloc (1,7);
            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((reader = read(fd, smb->pblock.words, 6)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Word_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            unsigned int *fid= calloc(1, 3); 

            if(fid == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(fid, smb->pblock.words, 2);

            unsigned int *bytes_to_be_read = calloc(1,2);

            if(bytes_to_be_read == NULL){

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(bytes_to_be_read, smb->pblock.words + 2, 2);


            unsigned int *estimate_bytes= calloc(1,3);

            if(estimate_bytes  == NULL){

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(estimate_bytes, smb->pblock.words + 4, 2);

            if(*estimate_bytes != 0){

                fprintf(stderr ,"[Validate_command] Estimate bytes incorrect\n");

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            printf("Fid: %x\n", *fid);
            printf("Bytes to read: %x\n", *bytes_to_be_read);
            printf("Estimate bytes: %x\n", *estimate_bytes);

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if((checker = SMB_COM_READ(fd, *fid)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(fid);
                fid = NULL;

                free(bytes_to_be_read);
                bytes_to_be_read = NULL;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                return smb;

            }

            break;

        case 10:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }
            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 5){

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word Count Check Passed\n");

            }

            reader = 0;
            smb->pblock.words = calloc(1, 7);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }


            if((reader = read(fd, smb->pblock.words, 6)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Words Error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            fid = calloc(1, 3); 

            if(fid == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(fid, smb->pblock.words, 2);

            unsigned int *bytes_to_write = calloc(1,2);

            if(bytes_to_write == NULL){

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(bytes_to_write, smb->pblock.words + 2, 2);

            estimate_bytes= calloc(1,3);

            if(estimate_bytes == NULL){

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            memcpy(estimate_bytes, smb->pblock.words + 4, 2);

            if(*estimate_bytes != 0){
                fprintf(stderr ,"[Validate_command] Estimate bytes incorrect\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            if(*bytes_to_write > MAX_BYTE_SIZE){
                fprintf(stderr ,"[Validate_command] Bytes to write is too large incorrect\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            printf("Fid: %x\n", *fid);
            printf("Bytes to write: %x\n", *bytes_to_write);
            printf("Estimate bytes: %x\n", *estimate_bytes);

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Byte Count Error\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 3){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 4);

            if(smb->dblock.bytes == NULL){

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->dblock.bytes , 3 + MAX_BYTE_SIZE)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((checker = SMB_COM_WRITE(fd, *fid, smb->dblock.bytes + 3 ,*bytes_to_write)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(estimate_bytes);
                estimate_bytes = NULL;

                free(bytes_to_write);
                bytes_to_write = NULL;

                free(fid);
                fid = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return smb;

            }

            break;

        case 11:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }


            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Reading Word Count Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->pblock.word_count[0] != 1){

                fprintf(stderr, "\n[SMB_PARSER] Word_Count Check Failed\n");

                smb->pblock.word_count[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Word_Count Check Passed\n");

            }

            reader = 0;
            smb->pblock.words = calloc(1,3);

            if(smb->pblock.words == NULL){

                free(smb);
                smb = NULL;

                return NULL;

            }

            if((reader = read(fd, smb->pblock.words, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Word_Count read error\n");

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            //FileAttr
            uint8_t *create_file_attr = calloc(1,2);

            if(create_file_attr == NULL){

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }


            memcpy(create_file_attr, smb->pblock.words, 2);

            int i = 0;
            fprintf(stderr, "\n[SMB_PARSER] File attr: ");
            for(i = 0; i < 2; ++i){
                fprintf(stderr, " %x " , *(create_file_attr + i));
            }

            fprintf(stderr, "\n");

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE+ 1);

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            //BufferFormat 0x04 SMB string, a null terminated string
            //Buffer Format is 2 bytes
            buffer_format = calloc(1,3);

            if(buffer_format == NULL){

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }


            strncpy(buffer_format, smb->dblock.bytes, 2);

            memcpy(&version, smb->dblock.bytes + 2, 8);

            //Directory Name
            path = calloc(1, MAX_BYTE_SIZE+ 1);

            if(path == NULL){

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            }

            strncpy(path, smb->dblock.bytes + 10, MAX_BYTE_SIZE);

            printf("Buffer format: %s\n" , buffer_format);
            printf("Version: 0x%" PRIx64 "\n" , version);
            printf("Path: %s\n" , path);

            if((ppm = SMB_COM_CREATE_NEW(fd, path,version)) == NULL){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                free(path);
                path = NULL;

                free(buffer_format);
                buffer_format = NULL;

                free(create_file_attr);
                create_file_attr = NULL;

                free(smb->pblock.words);
                smb->pblock.words = NULL;

            }

            break;

        case 12:

            reader = 0;
            if((reader = read(fd, smb->header.flags1, 1)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags1 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags1[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Failed\n");

                smb->header.flags1[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags1 Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.flags2, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Flags2 Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.flags2[0] != 0){

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Failed\n");

                smb->header.flags2[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Flags2 Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.PIDhigh, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDhigh Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDhigh[0] < 0 || smb->header.PIDhigh[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Failed\n");

                smb->header.PIDhigh[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDhigh Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.reserved, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading Reserved Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }


            if(smb->header.reserved[0] != 0x0000){

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Failed\n");

                smb->header.reserved[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] Reserved Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.TID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading TID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.TID[0]  < 0 || smb->header.TID[0] > MAX_TID){

                fprintf(stderr, "\n[SMB_PARSER] TID Check Failed\n");

                smb->header.TID[0] = 0;

                free(smb);
                smb = NULL;

                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] TID Check Passed\n\n");

            }


            reader = 0;
            if((reader = read(fd, smb->header.PIDlow, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading PIDlow Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.PIDlow[0] < 0 || smb->header.PIDlow[0] > MAX_PID){

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Failed\n");

                smb->header.PIDlow[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] PIDlow Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.UID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading UID error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.UID[0] < 0 || smb->header.UID[0] > MAX_UID){

                fprintf(stderr, "\n[SMB_PARSER] UID Check Failed\n");

                smb->header.UID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] UID Check Passed\n\n");

            }

            reader = 0;
            if((reader = read(fd, smb->header.MID, 2)) == -1){

                fprintf(stderr,"\n[SMB_PARSER] Reading MID Error\n");

                free(smb);
                smb = NULL;

                return NULL;
            }

            if(smb->header.MID[0] < 0 || smb->header.MID[0] > MAX_MID){

                fprintf(stderr, "\n[SMB_PARSER] MID Check Failed\n");

                smb->header.MID[0] = 0;

                free(smb);
                smb = NULL;


                return NULL;

            } else {

                fprintf(stderr, "\n[SMB_PARSER] MID Check Passed\n\n");

            }

            //Parameter Block
            reader = 0;
            if((reader = read(fd, smb->pblock.word_count, 1)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Word_Count read error\n");

                return NULL;
            }

            if(smb->pblock.word_count[0] != 0x00){

                fprintf(stderr, "\n[SMB_PARSER] Invalid Word_Count field for request\n");

                smb->pblock.word_count[0] = 0;

                return NULL;
            }

            //Data Block
            reader = 0;
            if((reader = read(fd, smb->dblock.byte_count, 2)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Byte_Count read error\n");

                return NULL;
            }

            if(smb->dblock.byte_count[0] < 2){

                fprintf(stderr, "\n[SMB_PARSER] Invalid byte count field for request\n");

                smb->dblock.byte_count[0] = 0;

                return NULL;
            }


            reader = 0;
            smb->dblock.bytes = calloc(1, MAX_BYTE_SIZE + 1);

            if((reader = read(fd, smb->dblock.bytes , MAX_BYTE_SIZE)) == -1){
                fprintf(stderr,"\n[SMB_PARSER] Bytes read error\n");

                free(smb->dblock.bytes);
                smb->dblock.bytes = NULL;

                return NULL;

            }

            if((checker = SMB_COM_NEGOTIATE(fd, smb, 0)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                return NULL;

            } else {

                return smb;

            }

            break;

        case 13:
            fprintf(stderr, "[SMB_PARSER] ANDX is ready to launch\n");

            if((checker = SMB_COM_SETUP_ANDX(fd, smb, 0)) == -1){

                fprintf(stderr, "[SMB_PARSER] Something crazy happened with the function\n");

                return NULL;

            } else if(checker == 0){

                fprintf(stderr, "[SMB_PARSER] There was an error in the the parameter or datablock\n");

                return NULL;

            } else {

                return smb;

            }

            break;

        default:
            fprintf(stderr, "[SMB_PARSER] There was an error where in the command check\n");
    }

    if(smb->dblock.bytes != NULL){
        if(strstr(smb->dblock.bytes, ".ppm") != NULL && ppm != NULL){

            free(smb->dblock.bytes);
            smb->dblock.bytes = NULL;

            ppm->fp(250, 87, ppm->pixels, version);

            destroy_ppm(ppm);


        } else if(strstr(smb->dblock.bytes, ".pgm") != NULL && pgm != NULL){

            free(smb->dblock.bytes);
            smb->dblock.bytes = NULL;

            destroy_pgm(pgm);

        } else {
            printf("No Print\n");
        }
    } 


    return smb;

}

void print_req_header(Req_Header rh){


    printf("========REQ_HEADER========\n");
    printf("Protocol:");
    int i = 0;
    for(i = 0; i < 4;++i){
        printf("%" PRIx8 " " , rh.protocol[i]);
    }

    printf("\n");

    printf("command: 0x");
    printf("%" PRIx8 "\n" , rh.command[0]);

    printf("Flags1: 0x");
    printf("%" PRIx8 "\n" , rh.flags1[0]);

    printf("Flags2: 0x");
    printf("%" PRIx16 "\n" , rh.flags2[0]);

    printf("PIDhigh: 0x");
    printf("%" PRIx16 "\n" , rh.PIDhigh[0]);

    printf("Reserved: 0x");
    printf("%" PRIx16 "\n" , rh.reserved[0]);

    printf("TID: 0x");
    printf("%" PRIx16 "\n" , rh.TID[0]);

    printf("PIDlow: 0x");
    printf("%" PRIx16 "\n" , rh.PIDlow[0]);

    printf("UID: 0x");
    printf("%" PRIx16 "\n" , rh.UID[0]);

    printf("MID: 0x");
    printf("%" PRIx16 "\n" , rh.MID[0]);
}

void print_param_block(Parameter_Block pb){
    printf("Word Count: 0x");
    printf("%" PRIx8 "\n" , pb.word_count[0]);

    if(pb.words == NULL){
        return;
    }

    int i;
    printf("Words: ");
    for(i = 0; i < pb.word_count[0]; ++i){

        printf("%" PRIx8 "\n" , pb.words[i]);

    }

    printf("\n");

}

void print_data_block(Data_Block db){
    printf("Byte Count: 0x");
    printf("%" PRIx8 "\n" , db.byte_count[0]);

    if(db.bytes == NULL){
        return;
    }

    int i;
    printf("Bytes: ");
    for(i = 0; i < db.byte_count[0]; ++i){

        printf("%c" ,(char) db.bytes + i);

    }

    printf("\n");

}

void print_smb_struct(SMB_Struct *smb){
    if(smb == NULL){
        fprintf(stderr,"[PRINT_SMB_STRUCT] Smb struct passed isn't allocated\n");
        return;
    }

    print_req_header(smb->header);

    print_param_block(smb->pblock);

    print_data_block(smb->dblock);
}
