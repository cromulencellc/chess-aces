#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "runtime/interpreter.h"
#include "rust.h"
#include "testbed.h"

#define README_FILENAME "/base_data/README.txt"

#define ALARM_SECONDS 60000
#define BACKLOG 10     // how many pending connections queue will hold

void handle(int fd);
int serve();
void run();

int client_fd_global;

#define INPUT_BUF_SIZE 8192
char input_buf[INPUT_BUF_SIZE];

int read_readme_file(char ** buf) {
    FILE * fh = fopen(README_FILENAME, "rb");
    if (fh == NULL) {
        fh = fopen(README_FILENAME, "rb");
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

int run_1() {
    char * source;
    int err = read_readme_file(&source);
    if (err) {
        return err;
    }

    printf("%s", source);
    free(source);
    fflush(stdout);
    return 0;
}

int run_2() {
    struct interpreter * interpreter = interpreter_create();

    while (1) {
        unsigned int i = 0;
        int input_complete = 0;

        while (i < (INPUT_BUF_SIZE - 1)) {
            char c;
            if (read(0, &c, 1) != 1) {
                input_complete = 1;
                break;
            }
            input_buf[i++] = c;
            if (c == ';') {
                break;
            }
        }

        if (i == (INPUT_BUF_SIZE - 1)) {
            panic("Received too much input.");
        }
        if (input_complete) {
            return 0;
        }

        input_buf[i] = '\0';

        struct list * tokens;
        if (lexer(input_buf, &tokens)) {
            printf("Lexer error\n");
            fflush(stdout);
            return -1;
        }

        struct list * statements = parse(tokens);

        list_delete(tokens);

        if (interpreter_run(interpreter, statements)) {
            printf("Interpreter error\n");
            fflush(stdout);
            return -1;
        }

        object_delete(statements);

        if (interpreter->exited) {
            break;
        }
    }

    interpreter_delete(interpreter);

    return 0;
}

void run() {
    printf("Welcome to the Very Fast Graph Database!\n");
    printf("1) Read the Readme\n");
    printf("2) Use the interactive console\n");
    printf("> ");
    fflush(stdout);
    int option;
    scanf("%d", &option);
    if (option == 1) {
        run_1();
    }
    else if (option == 2) {
        printf("Option 2\n");
        fflush(stdout);
        run_2();
    }
    else {
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

    const char * port = getenv("PORT");
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
        // now accept an incoming connection:
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_fd =
            accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        if (client_fd == -1) {
            fprintf(stderr, "Error on accept\n");
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;
        }

        int one = 1;
        setsockopt(client_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

        pid_t child_pid = fork();
        if (child_pid == 0) {
            handle(client_fd);
        }
    }
}

void clean_shutdown() {
    FILE * fh = fopen("/tmp/clean_shutdown", "wb");
    fclose(fh);
    close(0);
    close(1);
    close(2);
    shutdown(client_fd_global, SHUT_RDWR);
    close(client_fd_global);

    exit(0);
}

void handle(int client_fd) {
    close(0); /* close standard input  */
    close(1); /* close standard output */
    close(2); /* close standard error  */

    if (    (dup(client_fd) != 0)
         || (dup(client_fd) != 1)
         || (dup(client_fd) != 2)) {
        perror("error duplicating socket for stdin/stdout/stderr");
        exit(1);
    }

    client_fd_global = client_fd;

    alarm(ALARM_SECONDS);

    run();

    clean_shutdown();
}


int main(void)
{
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  return serve();
}
