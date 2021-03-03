#ifndef AUTHENTICATE_H
#define AUTHENTICATE_H
#include <stdbool.h>
#include "request.h"
#include "role_table.h"
bool send_authorization_request(FILE *out, Request req, RoleTable *rt);
#endif
