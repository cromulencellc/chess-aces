/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 *
 * This example demonstrates how to implement cookie authentication
 * and session management using Mongoose.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <shadow.h>

#include "db_plugin.h"
#include "mongoose.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static int s_sig_num = 0;
static void *s_db_handle = NULL;
static const char *s_db_path;
static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delete_method = MG_MK_STR("DELETE");

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);
  s_sig_num = sig_num;
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
  return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
  return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}
/* This is the name of the cookie carrying the session ID. */
#define SESSION_COOKIE_NAME "mgs"
/* In our example sessions are destroyed after 30 seconds of inactivity. */
#define SESSION_TTL 30.0
#define SESSION_CHECK_INTERVAL 15.0

/* Session information structure. */
struct session {
  /* Session ID. Must be unique and hard to guess. */
  uint64_t id;
  /*
   * Time when the session was created and time of last activity.
   * Used to clean up stale sessions.
   */
  double created;
  double last_used; /* Time when the session was last active. */

  /* User name this session is associated with. */
  char *user;
  /* Some state associated with user's session. */
  int lucky_number;
};

/*
 * Stores data about key value pairs
 */
char session_info[128];

/*
 * Logged in sessions
 */
#define NUM_SESSIONS 10
struct session s_sessions[NUM_SESSIONS];

/*
 * Password check function.
 */
static int check_pass(const char *user, const char *pass) {
  char *c = NULL;

  if ( !user || !pass ) {
    return 0;
  }

  struct spwd *pw = getspnam(user);

  if ( pw == NULL ) {
    return 0;
  }

  c = crypt( pass, pw->sp_pwdp);

  if ( c == NULL ) {
    return 0;
  }

  return (strcmp(c, pw->sp_pwdp) == 0);
}

/*
 * Parses the session cookie and returns a pointer to the session struct
 * or NULL if not found.
 */
static struct session *get_session(struct http_message *hm) {
  char ssid_buf[21];
  char *ssid = ssid_buf;
  struct session *ret = NULL;

  if ( hm == NULL ) {
    return NULL;
  }

  struct mg_str *cookie_header = mg_get_http_header(hm, "cookie");
  if (cookie_header == NULL) goto clean;
  if (!mg_http_parse_header2(cookie_header, SESSION_COOKIE_NAME, &ssid,
                             sizeof(ssid_buf))) {
    goto clean;
  }
  uint64_t sid = strtoull(ssid, NULL, 16);

  if ( sid == 0 ) {
    goto clean;
  }

  int i;
  for (i = 0; i < NUM_SESSIONS; i++) {
    if (s_sessions[i].id == sid) {
      s_sessions[i].last_used = mg_time();
      ret = &s_sessions[i];
      goto clean;
    }
  }

clean:
  if (ssid != ssid_buf) {
    free(ssid);
  }
  return ret;
}

/*
 * Scans the string for characters to ensure that is alpha numeric only
 * Returns 0 if the string is invalid, 1 otherwise
 */
static int check_bad_chars( const char *s) {
  int l;

  if ( s == NULL ) {
    return 0;
  }

  l = strlen(s);

  for ( int i = 0; i < l; i++) {
    if ( isalnum(s[i]) == 0) {
      return 0;
    }
  }

  return 1;
}

/*
 * Creates a new user on the system.
 * It is assumed that the string has already been sanitized
 */
static int create_user( const char *user, const char *pass) {
  if ( user == NULL ) {
    return 0;
  }

  char cmd[128];

  snprintf(cmd, sizeof(cmd), "useradd -ms /bin/bash %s\n", user);

  /// CWE-253: Incorrect Check of Function Return Value
  /// CWE-269: The software does not properly assign, modify, track, or check privileges for an actor, creating an unintended sphere of control for that actor.
#ifdef PATCH_1
  if ( system(cmd) ) {
    return 0;
  }
#else
  system(cmd);
#endif
  
  snprintf(cmd, sizeof(cmd), "echo %s:%s | chpasswd\n", user, pass);

  if ( system(cmd) ) { 
    return 0;
  }

  return 1;
}


/*
 * Destroys the session state.
 */
static void destroy_session(struct session *s) {
  if (!s) {
    return;
  }

  free(s->user);
  memset(s, 0, sizeof(*s));
}

/*
 * Creates a new session for the user.
 */
static struct session *create_session(const char *user,
                                      const struct http_message *hm) {
  /* Find first available slot or use the oldest one. */
  struct session *s = NULL;
  struct session *oldest_s = s_sessions;
  int i;

  if ( !user || !hm ) {
    return NULL;
  }

  for (i = 0; i < NUM_SESSIONS; i++) {
    if (s_sessions[i].id == 0) {
      s = &s_sessions[i];
      break;
    }
    if (s_sessions[i].last_used < oldest_s->last_used) {
      oldest_s = &s_sessions[i];
    }
  }
  if (s == NULL) {
    destroy_session(oldest_s);
    printf("Evicted %" INT64_X_FMT "/%s\n", oldest_s->id, oldest_s->user);
    s = oldest_s;
  }
  /* Initialize new session. */
  s->created = s->last_used = mg_time();
  s->user = strdup(user);

  if ( !s->user ) {
    return NULL;
  }

  s->lucky_number = rand();
  /* Create an ID by putting various volatiles into a pot and stirring. */
  cs_sha1_ctx ctx;
  cs_sha1_init(&ctx);
  cs_sha1_update(&ctx, (const unsigned char *) hm->message.p, hm->message.len);
  cs_sha1_update(&ctx, (const unsigned char *) s, sizeof(*s));
  unsigned char digest[20];
  cs_sha1_final(digest, &ctx);
  s->id = *((uint64_t *) digest);
  return s;
}

/*
 * If requested via GET, serves the login page.
 * If requested via POST (form submission), checks password and logs user in.
 */
static void login_handler(struct mg_connection *nc, int ev, void *p) {
  char user[50], pass[50];
  struct http_message *hm = (struct http_message *) p;

  if ( p == NULL) {
    mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
    return;
  }

  if (mg_vcmp(&hm->method, "POST") != 0) {
    /* Serve login.html */
    mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  } else {
    /* Perform password check. */
    int ul = mg_get_http_var(&hm->body, "user", user, sizeof(user));
    int pl = mg_get_http_var(&hm->body, "pass", pass, sizeof(pass));

    if (ul > 0 && pl > 0) {
      if (check_pass(user, pass)) {
        struct session *s = create_session(user, hm);
        char shead[100];
        snprintf(shead, sizeof(shead),
                 "Set-Cookie: %s=%" INT64_X_FMT "; path=/", SESSION_COOKIE_NAME,
                 s->id);
        mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(shead));
        fprintf(stderr, "%s logged in, sid %" INT64_X_FMT "\n", s->user, s->id);
      } else {
        mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\nWrong password.\r\n");
      }
    } else {
      mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
    }
    nc->flags |= MG_F_SEND_AND_CLOSE;
  }
  (void) ev;
}

/*
 * If requested via GET, serves the signup page.
 * If requested via POST (form submission), creates a new user with the given password.
 */
static void signup_handler(struct mg_connection *nc, int ev, void *p) {
  struct http_message *hm = (struct http_message *) p;

  if (p == NULL) {
    mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
    return;
  }

  if (mg_vcmp(&hm->method, "POST") != 0) {
    /* Serve signup.html */
    mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  } else {
    /* Get the username/password combination */
    char user[50], pass[50];
    int ul = mg_get_http_var(&hm->body, "user", user, sizeof(user));
    int pl = mg_get_http_var(&hm->body, "pass", pass, sizeof(pass));

    if (ul > 0 && pl > 0) {
      if ( !check_bad_chars(user) || !check_bad_chars(pass)) {
        mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\ninvalid input\r\n");
      } else {
        if ( !create_user( user, pass) ) {
          mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nunknown error\r\n");
        } else {
          mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(NULL));
        }
      }
    } else {
      mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
    }
    nc->flags |= MG_F_SEND_AND_CLOSE;
  }
  (void) ev;
}

/*
 * Logs the user out.
 * Removes cookie and any associated session state.
 */
static void logout_handler(struct mg_connection *nc, int ev, void *p) {
  struct http_message *hm = (struct http_message *) p;

  if (p == NULL) {
    mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }

  struct session *s = get_session(hm);
  if (s != NULL) {
    fprintf(stderr, "%s logged out, session %" INT64_X_FMT " destroyed\n",
            s->user, s->id);
    destroy_session(s);
  } else {
    char shead[100];
    mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n");
    snprintf(shead, sizeof(shead), "Connection: Close");
    mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(shead));
  }

  nc->flags |= MG_F_SEND_AND_CLOSE;
  (void) ev;
}

static void token_handler(struct mg_connection *nc, int ev, void *p) {
  if (p == NULL) {
    mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }
  
  struct http_message *hm = (struct http_message *) p;
  struct session *s = get_session(hm);

  if (s == NULL) {
    mg_http_send_redirect(nc, 302, mg_mk_str("/login.html"), mg_mk_str(NULL));
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }

  if ( strcmp( s->user, "chess") ) {
    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }

  nc->user_data = s;
  mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);

  nc->flags |= MG_F_SEND_AND_CLOSE;

  (void) p;
  (void) ev;
}

/* Cleans up sessions that have been idle for too long. */
void check_sessions(void) {
  double threshold = mg_time() - SESSION_TTL;
  int i;
  for (i = 0; i < NUM_SESSIONS; i++) {
    struct session *s = &s_sessions[i];
    if (s->id != 0 && s->last_used < threshold) {
      fprintf(stderr, "Session %" INT64_X_FMT " (%s) closed due to idleness.\n",
              s->id, s->user);
      destroy_session(s);
    }
  }
}

/* Main event handler. */
static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  static const struct mg_str api_prefix = MG_MK_STR("/api/v1");
  struct mg_str key;

  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) p;
      if (has_prefix(&hm->uri, &api_prefix)) {
        key.p = hm->uri.p + api_prefix.len;
        key.len = hm->uri.len - api_prefix.len;
        if (is_equal(&hm->method, &s_get_method)) {
          db_op(nc, hm, &key, s_db_handle, API_OP_GET);
        } else if (is_equal(&hm->method, &s_put_method)) {
          db_op(nc, hm, &key, s_db_handle, API_OP_SET);
        } else if (is_equal(&hm->method, &s_delete_method)) {
          db_op(nc, hm, &key, s_db_handle, API_OP_DEL);
        } else {
          mg_printf(nc, "%s",
                    "HTTP/1.0 501 Not Implemented\r\n"
                    "Content-Length: 0\r\n\r\n");
        }
      } else {
        struct session *s = get_session(hm);
        /* Ask the user to log in if they did not present a valid cookie. */
        if (s == NULL) {
          mg_http_send_redirect(nc, 302, mg_mk_str("/login.html"),
                                mg_mk_str(NULL));
          nc->flags |= MG_F_SEND_AND_CLOSE;
          break;
        }
        /*
         * Serve the page that was requested.
         * Save session in user_data for use by SSI calls.
         */
        nc->user_data = s;
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
      }
      break;
    }
    case MG_EV_SSI_CALL: {
      /* Expand variables in a page by using session data. */
      const char *var = (const char *) p;
      const struct session *s = (const struct session *) nc->user_data;
      if (strcmp(var, "user") == 0) {
        mg_printf_html_escape(nc, "%s", s->user);
      } else if (strcmp(var, "lucky_number") == 0) {
        mg_printf_html_escape(nc, "%d", s->lucky_number);
      }
      break;
    }
    case MG_EV_TIMER: {
      /* Perform session maintenance. */
      check_sessions();
      mg_set_timer(nc, mg_time() + SESSION_CHECK_INTERVAL);
      break;
    }
  }
}

int main(int argc, char **argv) {
  struct mg_mgr mgr;
  struct mg_connection *nc;
  srand(mg_time());

  char *root = getenv("WWWROOT");

  if (root == NULL) {
    root = "/var/www";
  }

  s_db_path = getenv("DBPATH");

  if ( s_db_path == NULL ) {
    s_db_path = "api_server.db";
  }

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);

  if ( nc == NULL ) {
    fprintf(stderr, "[ERROR] Failed to bind to port: %s\n", s_http_port);
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  /* Open database */
  if ((s_db_handle = db_open(s_db_path)) == NULL) {
    fprintf(stderr, "Cannot open DB [%s]\n", s_db_path);
    exit(EXIT_FAILURE);
  }

  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = root;
  mg_register_http_endpoint(nc, "/login.html", login_handler);
  mg_register_http_endpoint(nc, "/logout", logout_handler);
  mg_register_http_endpoint(nc, "/signup.html", signup_handler);
  mg_register_http_endpoint(nc, "/token", token_handler);
  s_http_server_opts.enable_directory_listing = "yes";
  mg_set_timer(nc, mg_time() + SESSION_CHECK_INTERVAL);

  printf("Starting web server on port %s\n", s_http_port);

  while (s_sig_num == 0) {
    mg_mgr_poll(&mgr, 500);
  }
  mg_mgr_free(&mgr);

  (void) argc;
  (void) argv;
  return 0;
}
