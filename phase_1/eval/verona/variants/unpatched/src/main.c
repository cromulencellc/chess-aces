#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"
#include "container/bb.h"
#include "instructions.h"
#include "runtime/jit.h"
#include "rust.h"
#include "testbed.h"
#define EXPLOIT_SIMULATOR_FILENAME "/data/exploit-simulator.s"
#define EXPLOIT_SIMULATOR_FILENAME2 "/base_data/exploit-simulator.s"
#define ALARM_SECONDS 30
#define BACKLOG 10
int handle(int fd);
int serve();
void run();
int read_exploit_simulator(char **buf) {
  FILE *fh = fopen(EXPLOIT_SIMULATOR_FILENAME, "rb");
  if (fh == NULL) {
    fh = fopen(EXPLOIT_SIMULATOR_FILENAME2, "rb");
    if (fh == NULL) {
      printf("Failed to open exploit simulator file\n");
      return -1;
    }
  }
  fseek(fh, 0, SEEK_END);
  size_t filesize = ftell(fh);
  fseek(fh, 0, SEEK_SET);
  *buf = malloc(filesize + 1);
  if (buf == NULL) {
    printf("Malloc file\n");
    return -1;
  }
  size_t bytes_read = fread(*buf, 1, filesize, fh);
  fclose(fh);
  if (bytes_read != filesize) {
    printf("Failed to read exploit simulator file\n");
    return -1;
  }
  (*buf)[filesize] = '\0';
  return 0;
}
int read_user_program(char **buf) {
  printf("Enter your program. Send \"END_PROGRAM\" without quotes when you are "
         "done entering your program\n");
  struct bb *bb = bb_create(1024 * 1024 * 4);
  unsigned int primed = 11;
  char last_chars[11];
  memset(last_chars, 0, 11);
  while (1) {
    uint8_t byte;
    int bytes_read = read(0, &byte, 1);
    if (bytes_read != 1) {
      printf("Error reading program\n");
      return -1;
    }
    if (primed == 0) {
      try
        (bb_push(bb, last_chars[0]))
    } else {
      primed--;
    }
    unsigned int i;
    for (i = 0; i < 10; i++) {
      last_chars[i] = last_chars[i + 1];
    }
    last_chars[10] = byte;
    if (memcmp(last_chars, "END_PROGRAM", 11) == 0) {
      break;
    }
  }
  *buf = malloc(bb_length(bb) + 1);
  memcpy(*buf, bb_data(bb), bb_length(bb));
  (*buf)[bb_length(bb)] = '\0';
  bb_delete(bb);
  return 0;
}
int run_1() {
  char *source;
  int err = read_exploit_simulator(&source);
  if (err) {
    return err;
  }
  printf("%s", source);
  free(source);
  fflush(stdout);
  return 0;
}
int run_2() {
  char *source;
  int err = read_exploit_simulator(&source);
  if (err) {
    return err;
  }
  struct list *tokens;
  try
    (lexer(source, &tokens)) struct binary *binary;
  try
    (parse(tokens, &binary))
        list_delete(tokens, (void (*)(void *))token_delete);
  free(source);
  struct context *context = context_create();
  if (context_load_binary(context, binary)) {
    fprintf(stderr, "Error loading binary");
    return -1;
  }
  binary_delete(binary);
  printf("Running\n");
  fflush(stdout);
  try
    (jit_run(context)) context_delete(context);
  return 0;
}
int run_3() {
  char *source;
  int err = read_user_program(&source);
  if (err) {
    return err;
  }
  struct list *tokens;
  try
    (lexer(source, &tokens)) struct binary *binary;
  try
    (parse(tokens, &binary))
        list_delete(tokens, (void (*)(void *))token_delete);
  free(source);
  printf("binary->code_size: %u\n", binary->code_size);
  printf("binary->data_size: %u\n", binary->data_size);
  printf("Code section: ");
  unsigned int i;
  for (i = 0; i < binary->code_size; i++) {
    printf("%02x", binary->code[i]);
  }
  printf("\n");
  printf("Data section: ");
  for (i = 0; i < binary->data_size; i++) {
    printf("%02x", binary->data[i]);
  }
  printf("\n");
  for (i = 0; i < binary->code_size;) {
    struct instruction instruction;
    try
      (instruction_decode(&instruction, i, &binary->code[i],
                          binary->code_size - i)) char buf[64];
    instruction_sprintf(buf, 64, &instruction);
    printf("0x%04x: (", i);
    unsigned int j;
    for (j = 0; j < 4; j++) {
      if (j < instruction.length) {
        printf("%02x", binary->code[i + j]);
      } else {
        printf("  ");
      }
    }
    printf(") %s\n", buf);
    i += instruction.length;
  }
  binary_delete(binary);
  return 0;
}
int run_4() {
  char *source;
  int err = read_user_program(&source);
  if (err) {
    return err;
  }
  struct list *tokens;
  try
    (lexer(source, &tokens)) struct binary *binary;
  try
    (parse(tokens, &binary))
        list_delete(tokens, (void (*)(void *))token_delete);
  free(source);
  struct context *context = context_create();
  if (context_load_binary(context, binary)) {
    fprintf(stderr, "Error loading binary");
    return -1;
  }
  binary_delete(binary);
  printf("Running\n");
  fflush(stdout);
  try
    (jit_run(context)) context_delete(context);
  return 0;
}
void run() {
  printf("Welcome to the Exploit Simulator!\n");
  printf("1) See the code for the exploit simulator program\n");
  printf("2) Run the exploit simulator\n");
  printf("3) Assemble and dump contents of your own program\n");
  printf("4) Run your own program\n");
  printf("> ");
  fflush(stdout);
  int option;
  scanf("%d", &option);
  fflush(stdout);
  if (option == 1) {
    run_1();
  } else if (option == 2) {
    run_2();
  } else if (option == 3) {
    run_3();
  } else if (option == 4) {
    run_4();
  } else {
    printf("Invalid option %d\n", option);
  }
}
int serve() {
  struct addrinfo hints;
  struct addrinfo *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  const char *port = getenv("PORT");
  if (port == NULL) {
    fprintf(stderr, "Missing environment variable PORT\n");
    return -1;
  }
  getaddrinfo(NULL, port, &hints, &res);
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  if (bind(sockfd, res->ai_addr, res->ai_addrlen)) {
    fprintf(stderr, "Error binding to socket\n");
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  if (listen(sockfd, BACKLOG)) {
    fprintf(stderr, "Error listening to bound socket\n");
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  while (1) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    printf("Waiting for accept\n");
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1) {
      fprintf(stderr, "Error on accept\n");
      fprintf(stderr, "%s\n", strerror(errno));
      return -1;
    }
    char host_[1024];
    char service[64];
    if (getnameinfo((const struct sockaddr *)&client_addr, addr_size, host_,
                    1024, service, 64, 0) == 0) {
      printf("Connection from %s:%s\n", host_, service);
    }
    pid_t child_pid = fork();
    if (child_pid == 0) {
      handle(client_fd);
    }
  }
}
int handle(int client_fd) {
  close(0);
  close(1);
  close(2);
  if ((dup(client_fd) != 0) || (dup(client_fd) != 1) || (dup(client_fd) != 2)) {
    perror("error duplicating socket for stdin/stdout/stderr");
    exit(1);
  }
  alarm(ALARM_SECONDS);
  run();
  shutdown(client_fd, SHUT_RDWR);
  close(client_fd);
  close(0);
  close(1);
  close(2);
  exit(0);
}
int main(void) { 
	#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
	return serve(); }
