#include <iostream>
#include "server.hpp"
#include "testbed.hpp"
int main(int argc, char **argv) {
  char *temp;
  int port;
  Server *s = NULL;
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  temp = getenv("PORT");
  if (temp == NULL) {
    std::cout << "[ERROR] environment variable PORT must be set" << std::endl;
    exit(1);
  }
  port = atoi(temp);
  s = new Server(port);
  s->SetLogging(1);
  s->Run();
  return 0;
}