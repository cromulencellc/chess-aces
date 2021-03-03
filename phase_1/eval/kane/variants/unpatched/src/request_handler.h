#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H
#include "role_table.h"
void request_handler(FILE *in, FILE *out, int fd, RoleTable *rt);
void kill_connection();
#endif
