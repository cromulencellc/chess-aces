#include "connection.hpp"
#include "server.hpp"
#include "testbed.hpp"
#include "user.hpp"
#include "utils.hpp"
using namespace std;
vector<connection *> conns;
extern std::vector<ftpuser *> users_g;
extern std::string userlist;
Server *srvr;
void usage(char *nm) {
  if (!nm) {
    exit(1);
  }
  printf("USAGE: %s -p <listen port> -u <userlist>\n", nm);
  printf("\tlisten port specifies on which port remsen listens. Can also be "
         "set via environment variable\n");
  printf("\t-u specifies the users allowed to log in along with their "
         "passwords\n");
  exit(1);
}
int stdin_only() {
  srvr = new Server();
  std::cout << "Not implemented" << std::endl;
  exit(1);
  return SUCCESS;
}
int with_network(int port) {
  srvr = new Server(port);
  srvr->select_loop();
  return SUCCESS;
}
int main(int argc, char **argv) {
  int port;
  char c;
  char *ul = NULL;
  int use_stdin = 0;
  int port_set = 0;
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  while ((c = getopt(argc, argv, "u:p:s")) != -1)
    switch (c) {
    case 'u':
      if (ul) {
        usage(argv[0]);
        fprintf(stderr, "Only one -u option permitted\n");
      }
      ul = strdup(optarg);
      if (ul == NULL) {
        return 1;
      }
      break;
    case 'p':
      port = atoi(optarg);
      port_set = 1;
      break;
    case 's':
      use_stdin = 1;
      break;
    case '?':
      if (optopt == 'p' || optopt == 'u') {
        fprintf(stderr, "-%c argument required\n", optopt);
        usage(argv[0]);
      } else {
        fprintf(stderr, "Unknown option\n");
        usage(argv[0]);
      }
    default:
      exit(1);
    }
  if (!ul) {
    ul = getenv("USERLIST");
    if (!ul) {
      ul = (char *)"userlist.txt";
    }
  }
  userlist = ul;
  if (parse_userlist(userlist) != SUCCESS) {
    cout << "Failed to parse userlist" << endl;
    return 0;
  }
  if (!port_set) {
    char *t = getenv("PORT");
    if (t != NULL) {
      port = atoi(t);
    } else {
      port = 3004;
    }
  }
  if (!use_stdin) {
    with_network(port);
  } else {
    stdin_only();
  }
  return 0;
}