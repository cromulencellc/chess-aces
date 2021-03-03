#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "StateMachine.h"
#include "testbed.hpp"
#define PORT (8888)
int main(int argc, char *argv[]) {
  int port = PORT;
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  const char *envPort = std::getenv("PORT");
  if (nullptr != envPort) {
    port = ::atoi(envPort);
  }
  bool shutdown = false;
  int activity, sd, maxSD;
  char readBuffer[1025];
  try {
    StateMachine state;
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == 0) {
      exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0) {
      exit(EXIT_FAILURE);
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
      exit(EXIT_FAILURE);
    }
    if (listen(server, 3) < 0) {
      exit(EXIT_FAILURE);
    }
    int addrlen = sizeof(address);
    fd_set readfds;
    while (!shutdown) {
      FD_ZERO(&readfds);
      FD_SET(server, &readfds);
      maxSD = server;
      for (int i = 0; i < MAX_LOGINS; i++) {
        sd = state.getClient(i);
        if (sd > 0) {
          FD_SET(sd, &readfds);
        }
        if (sd > maxSD) {
          maxSD = sd;
        }
      }
      activity = select(maxSD + 1, &readfds, NULL, NULL, NULL);
      if ((activity < 0) && (errno != EINTR)) {
        printf("select error");
      }
      if (FD_ISSET(server, &readfds)) {
        int newConnection =
            accept(server, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (newConnection < 0) {
          exit(EXIT_FAILURE);
        }
        state.insertClient(newConnection);
      }
      for (int i = 0; i < MAX_LOGINS; ++i) {
        sd = state.getClient(i);
        if (FD_ISSET(sd, &readfds)) {
          char *buf = (char *)malloc(sizeof(ControlMessage));
          if (buf) {
            memset(buf, 0x00, sizeof(ControlMessage));
            int amountRead = read(sd, buf, sizeof(ControlMessage));
            if (amountRead == 0) {
              getpeername(sd, (struct sockaddr *)&address,
                          (socklen_t *)&addrlen);
              close(sd);
              state.removeClient(i);
            } else {
              ControlMessage *cm = (ControlMessage *)buf;
              uint32_t msgSize = cm->messageSize;
              uint32_t amtRemaining = msgSize - amountRead;
              char *bufRemaining = (char *)realloc(buf, msgSize);
              if (bufRemaining) {
                buf = bufRemaining + sizeof(ControlMessage);
                while (amountRead < msgSize) {
                  uint32_t readRequest = (msgSize - amountRead > 1024)
                                             ? 1024
                                             : msgSize - amountRead;
                  uint32_t curRead = read(sd, buf, 1024);
                  amountRead += curRead;
                  buf += curRead;
                }
                if (amountRead != msgSize) {
                  std::cout << "Read Amount: " << amountRead
                            << " != " << msgSize << std::endl;
                }
                buf = bufRemaining;
                ControlMessage *reply =
                    state.processMessage((uint8_t *)buf, amountRead, sd);
                if (reply) {
                  free(buf);
                  buf = bufRemaining = NULL;
                  send(sd, reply, reply->messageSize, 0);
                  uint32_t replyType = reply->messageType;
                  free(reply);
                  reply = NULL;
                  if (replyType == SHUTDOWN_RESPONSE) {
                    shutdown = true;
                    for (int x = 0; x < MAX_LOGINS; ++x) {
                      int toClose = state.getClient(x);
                      if (toClose > 0) {
                        close(toClose);
                        state.removeClient(x);
                      }
                    }
                    break;
                  }
                }
              }
            }
          }
        }
      }
    }
  } catch (SystemException e) {
    std::cout << e;
    return -1;
  }
  return 0;
}
