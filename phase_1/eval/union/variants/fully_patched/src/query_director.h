#ifndef QUERY_DIRECTOR_H
#define QUERY_DIRECTOR_H
#include <stdbool.h>
#include "request.h"
#include "session.h"
bool direct_single_query(Request *req, char *query);
Session *direct_multi_query(Session *session, char *request);
#endif
