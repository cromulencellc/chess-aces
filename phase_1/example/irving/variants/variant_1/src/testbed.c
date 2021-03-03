#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "testbed.h"

#define TEST_BED_HOST "testbed_service_container"
#define TEST_BED_PORT 9494

#define FALSE 0
#define TRUE 1

void assert_execution_on_testbed()
{
    if (is_executing_on_testbed())
    {
        printf("Testbed detected.\n");
    }
    else
    {
        printf("Challenge is not executing on the testbed.\n");
        printf("Exiting.\n");
        exit(-1);
    }
}

void handle_container_stop_signals(int sig)
{
    printf("Caught container stop signal %d\n Goodbye \n", sig);
    exit(0);
}

int is_executing_on_testbed()
{
    
    const char *challenge = "Testbed_Challenge:1";

    
    struct sockaddr_in server_address;
    struct hostent *hostinfo;
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("%s:%d=>%s", __FILE__, __LINE__, strerror(errno));
        return FALSE;
    }
    memset(&server_address, '0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(TEST_BED_PORT);

    hostinfo = gethostbyname(TEST_BED_HOST);
    if (hostinfo == NULL)
    {
        printf("%s:%d=>%s", __FILE__, __LINE__, strerror(errno));
        return FALSE;
    }
    server_address.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("%s:%d=>%s", __FILE__, __LINE__, strerror(errno));
        return FALSE;
    }

    
    int send_error = send(sock, challenge, strlen(challenge), 0) < 0;
    if (send_error)
    {
        printf("%s:%d=>%s", __FILE__, __LINE__, strerror(errno));
        return FALSE;
    }

    
    char buffer[1024] = {0};
    int receive_error = recv(sock, buffer, 1024, 0) < 0;
    if (receive_error)
    {
        printf("%s:%d=>%s", __FILE__, __LINE__, strerror(errno));
        return FALSE;
    }

    
    
    char *result = strtok(buffer, "\n");
    int challenge_passed = !strcmp(result, challenge);
    if (!challenge_passed)
    {
        printf("Testbed service failed the challenge.\n");
    }
    return challenge_passed;
}
