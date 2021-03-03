#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "html_generation.h"
#include "query_director.h"
#include "request.h"
#include "response.h"
#include "session.h"
#define VALID_PATH "/lists"
void request_handler(FILE *in, FILE *out) {
  Request req = request_parse(in);
  if (req.valid == false) {
    req.error_number = 1;
    Response resp = assemble_response(&req, out);
    send_response(&resp, out);
    destroy_request(&req);
    destroy_response(&resp);
    return;
  }
  print_request(req);
  if (strncmp(req.path, VALID_PATH, strlen(req.path)) != 0) {
    req.valid = false;
    req.error_number = 2;
    Response resp = assemble_response(&req, out);
    send_response(&resp, out);
    destroy_request(&req);
    destroy_response(&resp);
    return;
  }
  Session *session = NULL;
  if (strncmp(req.method, "POST", strlen("POST")) == 0) {
    if (req.query != NULL) {
      char *holder;
      if ((holder = strstr(req.query, "session")) == NULL) {
        direct_single_query(&req, req.query);
        Response resp = assemble_response(&req, out);
        send_response(&resp, out);
        destroy_request(&req);
        destroy_response(&resp);
      } else {
        char *session_name;
        if ((session_name = strchr(holder, '=')) != NULL) {
          session_name += 1;
          session = load_session(session_name);
          print_master(session->mlist);
          session = direct_multi_query(session, req.body);
          if (!session->valid) {
            fprintf(stderr,
                    "[REQ_HANDLER] The session was processed to be invalid\n");
            return;
          }
          write_session(session);
          generate_html(session);
        }
        session = direct_multi_query(session, req.body);
        write_session(session);
        generate_html(session);
      }
    } else {
      fprintf(stderr,
              "[REQ_HANDLER] There was something wrong with the request\n");
      req.error_number = 1;
      Response resp = assemble_response(&req, out);
      send_response(&resp, out);
      destroy_session(session);
      destroy_request(&req);
      destroy_response(&resp);
    }
  } else if (strncmp(req.method, "GET", strlen("GET")) == 0) {
    Response resp = assemble_response(&req, out);
    printf("Body: %s\n", resp.entity_body);
    send_response(&resp, out);
    destroy_session(session);
    destroy_request(&req);
    destroy_response(&resp);
  } else {
    req.error_number = 3;
    Response resp = assemble_response(&req, out);
    send_response(&resp, out);
    destroy_request(&req);
    destroy_response(&resp);
  }
}
