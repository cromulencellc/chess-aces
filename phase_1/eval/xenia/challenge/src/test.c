#include <stdio.h>
#include <stdlib.h>

#include "container/list.h"
#include "runtime/interpreter.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/o/ast.h"
#include "rust.h"

// delete this
#include "graphdb/vertex.h"

char input_buf[4096];

void clean_shutdown() {
    exit(0);
}

int main() {
    struct interpreter * interpreter = interpreter_create();

    DEBUG("0x%lx", sizeof(struct vertex));

    while (1) {
        unsigned int i = 0;
        int input_complete = 0;
        while (i < 4095) {
            int c = getchar();
            if (c == EOF) {
                input_complete = 1;
                break;
            }
            input_buf[i++] = c;
            if (c == ';') {
                break;
            }
        }

        if (i == 4095) {
            panic("Received too much input. Commands must be < 4095 characters.");
        }
        if (input_complete) {
            break;
        }

        input_buf[i] = '\0';

        struct list * tokens;
        if (lexer(input_buf, &tokens)) {
            printf("Lexer error\n");
            break;
        }

        struct list * statements = parse(tokens);

        list_delete(tokens);

        if (interpreter_run(interpreter, statements)) {
            printf("Interpreter error\n");
            break;
        }

        object_delete(statements);

        if (interpreter->exited) {
            break;
        }
    }

    interpreter_delete(interpreter);

    return 0;
}