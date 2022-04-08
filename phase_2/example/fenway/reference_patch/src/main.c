#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <string.h>

#include "smb.h"

#define PORT 3445
#define FDS_COUNT 1
#define TIMEOUT 3

/*IMPORTANT THIS ONLY WORKS IF A PORT ENVIORMENT VARIABLE IS DEFINED*/

int main(){

    char *testbed = getenv("CHESS");

    if(testbed == NULL){
        printf("Failed testbed check\n");
        return 1;
    }

    /*
       SOCKET
       Sends a request to the Protocol stack for an id to mark socket
       Returns the id held by the integer socket_id

       1) AF_INET: Establishes IPv4
       2) SOCK_STREAM establishes a two connection node, via socket
       3) 0 indicates a single protocol
     */

    int socket_id = socket(AF_INET , SOCK_STREAM, 0);


    if(socket_id < 0){
        perror("Failure in socket creation\n");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    int sockopt = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt , sizeof(opt));
    if(sockopt != 0){
        perror("MAIN OPTIONS FAILED...\n");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    int socko = setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv , sizeof(tv));
    if(socko != 0){
        perror("MAIN OPTIONS FAILED...\n");
        exit(EXIT_FAILURE);
    }



    //Struct used for binding information to our socket
    struct sockaddr_in bindings;

    int port = atoi(getenv("PORT"));
    bindings.sin_family = AF_INET;
    bindings.sin_port = htons(port);
    bindings.sin_addr.s_addr = INADDR_ANY;

    int bind_var = bind(socket_id , (struct sockaddr *)&bindings , sizeof(bindings));


    if(bind_var != 0){
        perror("Failure in the main binding...\n");
        exit(EXIT_FAILURE);
    } 

    //Listens for incoming connections
    int listen_var = listen(socket_id, 0);

    if(listen_var != 0){
        printf("Failure in the main listening...\n");
        return 1;
    }

    struct sockaddr address;
    socklen_t address_len = sizeof(address);

    while(1){

        //Main file descriptor
        struct pollfd fds[FDS_COUNT];
        fds[0].fd = socket_id;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        printf("\nListening for incoming connections\n");
        //Negative for infinite timeout
        int ret;
        ret = poll(fds, FDS_COUNT, -1);

        if(fds[0].revents & POLLIN){
            int main_accept = accept(socket_id, &address, &address_len);

            SMB_Struct *success = NULL;

            if((success = SMB_parse(main_accept)) != NULL){
                fprintf(stderr, "[MAIN] Success!\n");

                free(success);
                success = NULL;
            } 

            sleep(1);
            close(main_accept);
        }
    }

    return 0;
}
