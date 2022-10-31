#include "first.h"

#include "base.h"
#include "buffer.h"
#include "network.h"
#include "log.h"
#include "rand.h"
#include "chunk.h"
#include "h2.h"             /* h2_send_1xx() */
#include "http_auth.h"      /* http_auth_dumbdata_reset() */
#include "http_vhostdb.h"   /* http_vhostdb_dumbdata_reset() */
#include "fdevent.h"
#include "connections.h"
#include "sock_addr.h"
#include "stat_cache.h"
#include "plugin.h"
#include "plugin_config.h"  /* config_plugin_value_tobool() */
#include "network_write.h"  /* network_write_show_handlers() */
#include "reqpool.h"        /* request_pool_init() request_pool_free() */
#include "response.h"       /* http_response_send_1xx_cb_set() strftime_cache_reset() */

#ifdef HAVE_VERSIONSTAMP_H
# include "versionstamp.h"
#else
# define REPO_VERSION ""
#endif

#define PACKAGE_DESC PACKAGE_NAME "/" PACKAGE_VERSION REPO_VERSION
static const buffer default_server_tag = { CONST_STR_LEN(PACKAGE_DESC)+1, 0 };

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <locale.h>

#include <stdio.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifdef HAVE_VALGRIND_VALGRIND_H
# include <valgrind/valgrind.h>
#endif

#ifdef HAVE_PWD_H
# include <grp.h>
# include <pwd.h>
#endif

#ifdef HAVE_SYS_LOADAVG_H
# include <sys/loadavg.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#ifdef HAVE_SYS_PRCTL_H
# include <sys/prctl.h>
#endif

#include "sys-crypto.h"
#if defined(USE_OPENSSL_CRYPTO) \
 || defined(USE_MBEDTLS_CRYPTO) \
 || defined(USE_NSS_CRYPTO) \
 || defined(USE_GNUTLS_CRYPTO) \
 || defined(USE_WOLFTLS_CRYPTO)
#define TEXT_SSL " (ssl)"
#else
#define TEXT_SSL
#endif

#ifndef __sgi
/* IRIX doesn't like the alarm based time() optimization */
/* #define USE_ALARM */
#endif

static int oneshot_fd = 0;
static int oneshot_fdout = -1;
static fdnode *oneshot_fdn = NULL;
static int (*oneshot_read_cq)(connection *con, chunkqueue *cq, off_t max_bytes);
static volatile int pid_fd = -2;
static server_socket_array graceful_sockets;
static server_socket_array inherited_sockets;
static volatile sig_atomic_t graceful_restart = 0;
static volatile sig_atomic_t graceful_shutdown = 0;
static volatile sig_atomic_t srv_shutdown = 0;
static volatile sig_atomic_t handle_sig_child = 0;
static volatile sig_atomic_t handle_sig_alarm = 1;
static volatile sig_atomic_t handle_sig_hup = 0;
static time_t idle_limit = 0;

#if defined(HAVE_SIGACTION) && defined(SA_SIGINFO)
static volatile siginfo_t last_sigterm_info;
static volatile siginfo_t last_sighup_info;

static void sigaction_handler(int sig, siginfo_t *si, void *context) {
	static const siginfo_t empty_siginfo;
	UNUSED(context);

	if (!si) *(const siginfo_t **)&si = &empty_siginfo;

	switch (sig) {
	case SIGTERM:
		srv_shutdown = 1;
		last_sigterm_info = *si;
		break;
	case SIGUSR1:
		if (!graceful_shutdown) {
			graceful_restart = 1;
			graceful_shutdown = 1;
			last_sigterm_info = *si;
		}
		break;
	case SIGINT:
		if (graceful_shutdown) {
			if (2 == graceful_restart)
				graceful_restart = 1;
			else
				srv_shutdown = 1;
		} else {
			graceful_shutdown = 1;
		}
		last_sigterm_info = *si;

		break;
	case SIGALRM: 
		handle_sig_alarm = 1; 
		break;
	case SIGHUP:
		handle_sig_hup = 1;
		last_sighup_info = *si;
		break;
	case SIGCHLD:
		handle_sig_child = 1;
		break;
	}
}
#elif defined(HAVE_SIGNAL) || defined(HAVE_SIGACTION)
static void signal_handler(int sig) {
	switch (sig) {
	case SIGTERM: srv_shutdown = 1; break;
	case SIGUSR1:
		if (!graceful_shutdown) {
			graceful_restart = 1;
			graceful_shutdown = 1;
		}
		break;
	case SIGINT:
		if (graceful_shutdown) {
			if (2 == graceful_restart)
				graceful_restart = 1;
			else
				srv_shutdown = 1;
		} else {
			graceful_shutdown = 1;
		}
		break;
	case SIGALRM: handle_sig_alarm = 1; break;
	case SIGHUP:  handle_sig_hup = 1; break;
	case SIGCHLD: handle_sig_child = 1; break;
	}
}
#endif

#ifdef HAVE_FORK
static int daemonize(void) {
	int pipefd[2];
	pid_t pid;
#ifdef SIGTTOU
	signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif

	if (pipe(pipefd) < 0) exit(-1);

	if (0 > (pid = fork())) exit(-1);

	if (0 < pid) {
		char buf;
		ssize_t bytes;

		close(pipefd[1]);
		/* parent waits for grandchild to be ready */
		do {
			bytes = read(pipefd[0], &buf, sizeof(buf));
		} while (bytes < 0 && EINTR == errno);
		close(pipefd[0]);

		if (bytes <= 0) {
			/* closed fd (without writing) == failure in grandchild */
			fputs("daemonized server failed to start; check error log for details\n", stderr);
			exit(-1);
		}

		exit(0);
	}

	close(pipefd[0]);

	if (-1 == setsid()) exit(0);

	signal(SIGHUP, SIG_IGN);

	if (0 != fork()) exit(0);

	if (0 != chdir("/")) exit(0);

	fdevent_setfd_cloexec(pipefd[1]);
	return pipefd[1];
}
#endif

__attribute_cold__
static server *server_init(void) {
	server *srv = calloc(1, sizeof(*srv));
	force_assert(srv);
#define CLEAN(x) \
	srv->x = buffer_init();

	CLEAN(tmp_buf);
#undef CLEAN
	connection_joblist = &srv->joblist_A;

	strftime_cache_reset();

	li_rand_reseed();

	srv->startup_ts = log_epoch_secs = time(NULL);

	srv->errh = log_error_st_init();

	config_init(srv);

	srv->request_env = plugins_call_handle_request_env;

	srv->loadavg[0] = 0.0;
	srv->loadavg[1] = 0.0;
	srv->loadavg[2] = 0.0;
	srv->stdin_fd = -1;

	return srv;
}

__attribute_cold__
static void server_free(server *srv) {
	if (oneshot_fd > 0) {
		if (oneshot_fdn) {
			fdevent_fdnode_event_del(srv->ev, oneshot_fdn);
			fdevent_unregister(srv->ev, oneshot_fd);
			oneshot_fdn = NULL;
		}
		close(oneshot_fd);
	}
	if (oneshot_fdout >= 0) {
		close(oneshot_fdout);
	}
	if (srv->stdin_fd >= 0) {
		close(srv->stdin_fd);
	}

#define CLEAN(x) \
	buffer_free(srv->x);

	CLEAN(tmp_buf);

#undef CLEAN

	fdevent_free(srv->ev);

	config_free(srv);

	free(srv->joblist_A.ptr);
	free(srv->joblist_B.ptr);
	free(srv->fdwaitqueue.ptr);

	stat_cache_free();

	li_rand_cleanup();
	chunkqueue_chunk_pool_free();

	log_error_st_free(srv->errh);
	free(srv);
}

__attribute_cold__
static void remove_pid_file(server *srv) {
	if (pid_fd <= -2) return;
	if (!buffer_string_is_empty(srv->srvconf.pid_file) && 0 <= pid_fd) {
		if (0 != ftruncate(pid_fd, 0)) {
			log_perror(srv->errh, __FILE__, __LINE__,
			  "ftruncate failed for: %s", srv->srvconf.pid_file->ptr);
		}
	}
	if (0 <= pid_fd) {
		close(pid_fd);
		pid_fd = -1;
	}
	if (!buffer_string_is_empty(srv->srvconf.pid_file) &&
	    buffer_string_is_empty(srv->srvconf.changeroot)) {
		if (0 != unlink(srv->srvconf.pid_file->ptr)) {
			if (errno != EACCES && errno != EPERM) {
				log_perror(srv->errh, __FILE__, __LINE__,
				  "unlink failed for: %s", srv->srvconf.pid_file->ptr);
			}
		}
	}
}


__attribute_cold__
static server_socket * server_oneshot_getsock(server *srv, sock_addr *cnt_addr) {
	server_socket *srv_socket, *srv_socket_wild = NULL;
	for (uint32_t i = 0; i < srv->srv_sockets.used; ++i) {
		srv_socket = srv->srv_sockets.ptr[i];
		if (!sock_addr_is_port_eq(&srv_socket->addr,cnt_addr)) continue;
		if (sock_addr_is_addr_eq(&srv_socket->addr,cnt_addr)) return srv_socket;

		if (NULL != srv_socket_wild) continue;
		if (sock_addr_is_addr_wildcard(&srv_socket->addr)) {
			srv_socket_wild = srv_socket;
		}
	}

	if (NULL != srv_socket_wild) {
		return srv_socket_wild;
	} else if (srv->srv_sockets.used) {
		return srv->srv_sockets.ptr[0];
	} else {
		log_error(srv->errh, __FILE__, __LINE__, "no sockets configured");
		return NULL;
	}
}


static int server_oneshot_read_cq(connection *con, chunkqueue *cq, off_t max_bytes) {
    /* temporary set con->fd to oneshot_fd (fd input) rather than outshot_fdout
     * (lighttpd generally assumes operation on sockets, so this is a kludge) */
    int fd = con->fd;
    con->fd = oneshot_fdn->fd;
    int rc = oneshot_read_cq(con, cq, max_bytes);
    con->fd = fd;

    /* note: optimistic reads (elsewhere) may or may not be enough to re-enable
     * read interest after FDEVENT_IN interest was paused for other reasons */

    const int events = fdevent_fdnode_interest(oneshot_fdn);
    int n = con->is_readable > 0 ? 0 : FDEVENT_IN;
    if (events & FDEVENT_RDHUP)
        n |= FDEVENT_RDHUP;
    fdevent_fdnode_event_set(con->srv->ev, oneshot_fdn, n);
    return rc;
}


static handler_t server_oneshot_handle_fdevent(void *context, int revents) {
    connection *con = context;

    /* note: not sync'd with con->fdn or connection_set_fdevent_interest() */
    int rdhup = 0;
    int n = fdevent_fdnode_interest(oneshot_fdn);
    if (revents & FDEVENT_IN)
        n &= ~FDEVENT_IN;
    request_st * const r = &con->request;
    if (r->state != CON_STATE_ERROR && (revents & (FDEVENT_HUP|FDEVENT_RDHUP))){
        revents &= ~(FDEVENT_HUP|FDEVENT_RDHUP);
        /* copied and modified from connection_handle_fdevent()
         * fdevent_is_tcp_half_closed() will fail on pipe
         * and, besides, read end of pipes should treat POLLHUP as POLLRDHUP */
        n &= ~(FDEVENT_IN|FDEVENT_RDHUP);
        rdhup = 1;
    }
    fdevent_fdnode_event_set(con->srv->ev, oneshot_fdn, n);

    fdnode * const fdn = con->fdn; /* fdn->ctx == con */
    handler_t rc = ((fdevent_handler)NULL != fdn->handler)
      ? (*fdn->handler)(con, revents)
      : HANDLER_FINISHED;

    if (rdhup) {
        r->conf.stream_request_body &=
          ~(FDEVENT_STREAM_REQUEST_BUFMIN|FDEVENT_STREAM_REQUEST_POLLIN);
        r->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_POLLRDHUP;
        r->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_TCP_FIN;
        con->is_readable = 1; /*(can read 0 for end-of-stream)*/
        if (chunkqueue_is_empty(con->read_queue)) r->keep_alive = 0;
        if (r->reqbody_length < -1) /*(transparent proxy mode; no more data)*/
            r->reqbody_length = r->reqbody_queue.bytes_in;
    }

    return rc;
}


__attribute_cold__
static int server_oneshot_init_pipe(server *srv, int fdin, int fdout) {
    /* Note: attempt to work with netcat pipes though other code expects socket.
     * netcat has different fds (pipes) for stdin and stdout.  To support
     * netcat, need to avoid S_ISSOCK(), getsockname(), and getpeername(),
     * reconstructing addresses from environment variables:
     *   NCAT_LOCAL_ADDR   NCAT_LOCAL_PORT
     *   NCAT_REMOTE_ADDR  NCAT_REMOTE_PORT
     *   NCAT_PROTO (TCP, UDP, SCTP)
     */
    connection *con;
    server_socket *srv_socket;
    sock_addr cnt_addr;

    /* detect if called from netcat or else fabricate localhost addrs */
    const char * const ncat =
             getenv("NCAT_LOCAL_ADDR");
    const char * const ncat_local_addr  =
      ncat ? ncat                       : "127.0.0.1"; /*(fabricated)*/
    const char * const ncat_local_port  =
      ncat ? getenv("NCAT_LOCAL_PORT")  : "80";        /*(fabricated)*/
    const char * const ncat_remote_addr =
      ncat ? getenv("NCAT_REMOTE_ADDR") : "127.0.0.1"; /*(fabricated)*/
    const char * const ncat_remote_port =
      ncat ? getenv("NCAT_REMOTE_PORT") : "48080";     /*(fabricated)*/
    if (NULL == ncat_local_addr  || NULL == ncat_local_port)  return 0;
    if (NULL == ncat_remote_addr || NULL == ncat_remote_port) return 0;

    const int family = ncat && strchr(ncat_local_addr,':') ? AF_INET6 : AF_INET;
    unsigned short port;

    port = (unsigned short)strtol(ncat_local_port, NULL, 10);
    if (1 != sock_addr_inet_pton(&cnt_addr, ncat_local_addr, family, port)) {
        log_error(srv->errh, __FILE__, __LINE__, "invalid local addr");
        return 0;
    }

    srv_socket = server_oneshot_getsock(srv, &cnt_addr);
    if (NULL == srv_socket) return 0;

    port = (unsigned short)strtol(ncat_remote_port, NULL, 10);
    if (1 != sock_addr_inet_pton(&cnt_addr, ncat_remote_addr, family, port)) {
        log_error(srv->errh, __FILE__, __LINE__, "invalid remote addr");
        return 0;
    }

    /*(must set flags; fd did not pass through fdevent accept() logic)*/
    if (-1 == fdevent_fcntl_set_nb_cloexec(fdin)) {
        log_perror(srv->errh, __FILE__, __LINE__, "fcntl()");
        return 0;
    }
    if (-1 == fdevent_fcntl_set_nb_cloexec(fdout)) {
        log_perror(srv->errh, __FILE__, __LINE__, "fcntl()");
        return 0;
    }

    con = connection_accepted(srv, srv_socket, &cnt_addr, fdout);
    if (NULL == con) return 0;

    /* note: existing routines assume socket, not pipe
     * connections.c:connection_read_cq()
     *   uses recv() ifdef __WIN32
     *   passes S_IFSOCK to fdevent_ioctl_fionread()
     *   (The routine could be copied and modified, if required)
     * This is unlikely to work if TLS is used over pipe since the SSL_CTX
     * is associated with the other end of the pipe.  However, if using
     * pipes, using TLS is unexpected behavior.
     */

    /*assert(oneshot_fd == fdin);*/
    oneshot_read_cq = con->network_read;
    con->network_read = server_oneshot_read_cq;
    oneshot_fdn =
      fdevent_register(srv->ev, fdin, server_oneshot_handle_fdevent, con);
    fdevent_fdnode_event_set(srv->ev, oneshot_fdn, FDEVENT_RDHUP);

    connection_state_machine(con);
    return 1;
}


__attribute_cold__
static int server_oneshot_init(server *srv, int fd) {
	connection *con;
	server_socket *srv_socket;
	sock_addr cnt_addr;
	socklen_t cnt_len;

	cnt_len = sizeof(cnt_addr);
	if (0 != getsockname(fd, (struct sockaddr *)&cnt_addr, &cnt_len)) {
		log_perror(srv->errh, __FILE__, __LINE__, "getsockname()");
		return 0;
	}

	srv_socket = server_oneshot_getsock(srv, &cnt_addr);
	if (NULL == srv_socket) return 0;

      #ifdef __clang_analyzer__
        memset(&cnt_addr, 0, sizeof(cnt_addr));
      #endif
	cnt_len = sizeof(cnt_addr);
	if (0 != getpeername(fd, (struct sockaddr *)&cnt_addr, &cnt_len)) {
		log_perror(srv->errh, __FILE__, __LINE__, "getpeername()");
		return 0;
	}

	/*(must set flags; fd did not pass through fdevent accept() logic)*/
	if (-1 == fdevent_fcntl_set_nb_cloexec(fd)) {
		log_perror(srv->errh, __FILE__, __LINE__, "fcntl()");
		return 0;
	}

	if (sock_addr_get_family(&cnt_addr) != AF_UNIX) {
		network_accept_tcp_nagle_disable(fd);
	}

	con = connection_accepted(srv, srv_socket, &cnt_addr, fd);
	if (NULL == con) return 0;

	connection_state_machine(con);
	return 1;
}


__attribute_cold__
static void show_version (void) {
	char *b = PACKAGE_DESC TEXT_SSL \
" - a light and fast webserver\n"
#ifdef NONREPRODUCIBLE_BUILD
"Build-Date: " __DATE__ " " __TIME__ "\n";
#endif
;
	write_all(STDOUT_FILENO, b, strlen(b));
}

__attribute_cold__
static void show_features (void) {
  static const char features[] =
      "\nFeatures:\n\n"
#ifdef HAVE_IPV6
      "\t+ IPv6 support\n"
#else
      "\t- IPv6 support\n"
#endif
#if defined HAVE_ZLIB_H && defined HAVE_LIBZ
      "\t+ zlib support\n"
#else
      "\t- zlib support\n"
#endif
#if defined HAVE_ZSTD_H && defined HAVE_ZSTD
      "\t+ zstd support\n"
#else
      "\t- zstd support\n"
#endif
#if defined HAVE_BZLIB_H && defined HAVE_LIBBZ2
      "\t+ bzip2 support\n"
#else
      "\t- bzip2 support\n"
#endif
#if defined HAVE_BROTLI_ENCODE_H && defined HAVE_BROTLI
      "\t+ brotli support\n"
#else
      "\t- brotli support\n"
#endif
#if defined(HAVE_CRYPT) || defined(HAVE_CRYPT_R) || defined(HAVE_LIBCRYPT)
      "\t+ crypt support\n"
#else
      "\t- crypt support\n"
#endif
#ifdef USE_OPENSSL_CRYPTO
      "\t+ OpenSSL support\n"
#else
      "\t- OpenSSL support\n"
#endif
#ifdef USE_MBEDTLS_CRYPTO
      "\t+ mbedTLS support\n"
#else
      "\t- mbedTLS support\n"
#endif
#ifdef USE_NSS_CRYPTO
      "\t+ NSS crypto support\n"
#else
      "\t- NSS crypto support\n"
#endif
#ifdef USE_GNUTLS_CRYPTO
      "\t+ GnuTLS support\n"
#else
      "\t- GnuTLS support\n"
#endif
#ifdef USE_WOLFSSL_CRYPTO
      "\t+ WolfSSL support\n"
#else
      "\t- WolfSSL support\n"
#endif
#ifdef USE_NETTLE_CRYPTO
      "\t+ Nettle support\n"
#else
      "\t- Nettle support\n"
#endif
#ifdef HAVE_LIBPCRE
      "\t+ PCRE support\n"
#else
      "\t- PCRE support\n"
#endif
#ifdef HAVE_MYSQL
      "\t+ MySQL support\n"
#else
      "\t- MySQL support\n"
#endif
#ifdef HAVE_PGSQL
      "\t+ PgSQL support\n"
#else
      "\t- PgSQL support\n"
#endif
#ifdef HAVE_DBI
      "\t+ DBI support\n"
#else
      "\t- DBI support\n"
#endif
#ifdef HAVE_KRB5
      "\t+ Kerberos support\n"
#else
      "\t- Kerberos support\n"
#endif
#if defined(HAVE_LDAP_H) && defined(HAVE_LBER_H) && defined(HAVE_LIBLDAP) && defined(HAVE_LIBLBER)
      "\t+ LDAP support\n"
#else
      "\t- LDAP support\n"
#endif
#ifdef HAVE_PAM
      "\t+ PAM support\n"
#else
      "\t- PAM support\n"
#endif
#ifdef USE_MEMCACHED
      "\t+ memcached support\n"
#else
      "\t- memcached support\n"
#endif
#ifdef HAVE_FAM_H
      "\t+ FAM support\n"
#else
      "\t- FAM support\n"
#endif
#ifdef HAVE_LUA_H
      "\t+ LUA support\n"
#else
      "\t- LUA support\n"
#endif
#ifdef HAVE_LIBXML_H
      "\t+ xml support\n"
#else
      "\t- xml support\n"
#endif
#ifdef HAVE_SQLITE3_H
      "\t+ SQLite support\n"
#else
      "\t- SQLite support\n"
#endif
#ifdef HAVE_GDBM_H
      "\t+ GDBM support\n"
#else
      "\t- GDBM support\n"
#endif
      ;
  show_version();
  printf("%s%s%s\n", fdevent_show_event_handlers(), network_write_show_handlers(), features);
}

__attribute_cold__
static void show_help (void) {
	char *b = PACKAGE_DESC TEXT_SSL
#ifdef NONREPRODUCIBLE_BUILD
" ("__DATE__ " " __TIME__ ")"
#endif
" - a light and fast webserver\n" \
"usage:\n" \
" -f <name>  filename of the config-file\n" \
" -m <name>  module directory (default: "LIBRARY_DIR")\n" \
" -i <secs>  graceful shutdown after <secs> of inactivity\n" \
" -1         process single (one) request on stdin socket, then exit\n" \
" -p         print the parsed config-file in internal form, and exit\n" \
" -t         test config-file syntax, then exit\n" \
" -tt        test config-file syntax, load and init modules, then exit\n" \
" -D         don't go to background (default: go to background)\n" \
" -v         show version\n" \
" -V         show compile-time features\n" \
" -h         show this help\n" \
"\n"
;
	write_all(STDOUT_FILENO, b, strlen(b));
}

__attribute_cold__
static void server_sockets_save (server *srv) {    /* graceful_restart */
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i)
        srv->srv_sockets.ptr[i]->srv = NULL; /* srv will shortly be invalid */
    for (uint32_t i = 0; i < srv->srv_sockets_inherited.used; ++i)
        srv->srv_sockets_inherited.ptr[i]->srv = NULL; /* srv to be invalid */
    memcpy(&graceful_sockets, &srv->srv_sockets, sizeof(server_socket_array));
    memset(&srv->srv_sockets, 0, sizeof(server_socket_array));
    memcpy(&inherited_sockets, &srv->srv_sockets_inherited, sizeof(server_socket_array));
    memset(&srv->srv_sockets_inherited, 0, sizeof(server_socket_array));
}

__attribute_cold__
static void server_sockets_restore (server *srv) { /* graceful_restart */
    memcpy(&srv->srv_sockets, &graceful_sockets, sizeof(server_socket_array));
    memset(&graceful_sockets, 0, sizeof(server_socket_array));
    memcpy(&srv->srv_sockets_inherited, &inherited_sockets, sizeof(server_socket_array));
    memset(&inherited_sockets, 0, sizeof(server_socket_array));
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i)
        srv->srv_sockets.ptr[i]->srv = srv;           /* update ptr */
    for (uint32_t i = 0; i < srv->srv_sockets_inherited.used; ++i)
        srv->srv_sockets_inherited.ptr[i]->srv = srv; /* update ptr */
}

__attribute_cold__
static int server_sockets_set_nb_cloexec (server *srv) {
    if (srv->sockets_disabled) return 0; /* lighttpd -1 (one-shot mode) */
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i) {
        server_socket *srv_socket = srv->srv_sockets.ptr[i];
        if (-1 == fdevent_fcntl_set_nb_cloexec_sock(srv_socket->fd)) {
            log_perror(srv->errh, __FILE__, __LINE__, "fcntl()");
            return -1;
        }
    }
    return 0;
}

__attribute_cold__
static void server_sockets_set_event (server *srv, int event) {
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i) {
        server_socket *srv_socket = srv->srv_sockets.ptr[i];
        fdevent_fdnode_event_set(srv->ev, srv_socket->fdn, event);
    }
}

__attribute_cold__
static void server_sockets_unregister (server *srv) {
    if (2 == srv->sockets_disabled) return;
    srv->sockets_disabled = 2;
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i)
        network_unregister_sock(srv, srv->srv_sockets.ptr[i]);
}

__attribute_cold__
static void server_sockets_close (server *srv) {
    /* closing socket right away will make it possible for the next lighttpd
     * to take over (old-style graceful restart), but only if backends
     * (e.g. fastcgi, scgi, etc) are independent from lighttpd, rather
     * than started by lighttpd via "bin-path")
     */
    if (3 == srv->sockets_disabled) return;
    for (uint32_t i = 0; i < srv->srv_sockets.used; ++i) {
        server_socket *srv_socket = srv->srv_sockets.ptr[i];
        if (-1 == srv_socket->fd) continue;
        if (2 != srv->sockets_disabled) network_unregister_sock(srv,srv_socket);
        close(srv_socket->fd);
        srv_socket->fd = -1;
        /* network_close() will cleanup after us */
    }
    srv->sockets_disabled = 3;
}

__attribute_cold__
static void server_graceful_signal_prev_generation (void)
{
    const char * const prev_gen = getenv("LIGHTTPD_PREV_GEN");
    if (NULL == prev_gen) return;
    pid_t pid = (pid_t)strtol(prev_gen, NULL, 10);
    unsetenv("LIGHTTPD_PREV_GEN");
    if (pid <= 0) return; /*(should not happen)*/
    if (pid == fdevent_waitpid(pid,NULL,1)) return; /*(pid exited; unexpected)*/
    kill(pid, SIGINT); /* signal previous generation for graceful shutdown */
}

__attribute_cold__
static int server_graceful_state_bg (server *srv) {
    /*assert(graceful_restart);*/
    /*(SIGUSR1 set to SIG_IGN in workers, so should not reach here if worker)*/
    if (srv_shutdown) return 0;
    if (NULL == srv->srvconf.feature_flags) return 0;

    /* check if server should fork and background (bg) itself
     * to continue processing requests already in progress */
    data_unset * const du =
      array_get_data_unset(srv->srvconf.feature_flags,
                           CONST_STR_LEN("server.graceful-restart-bg"));
    if (!config_plugin_value_tobool(du, 0)) return 0;

    /*(set flag to false to avoid repeating)*/
    if (du->type == TYPE_STRING)
        buffer_copy_string_len(&((data_string *)du)->value,
                               CONST_STR_LEN("false"));
    else /* (du->type == TYPE_INTEGER) */
        ((data_integer *)du)->value = 0;

    /* require exec'd via absolute path or daemon in foreground
     * and exec'd with path containing '/' (e.g. "./xxxxx") */
    char ** const argv = srv->argv;
    if (0 == srv->srvconf.dont_daemonize
        ? argv[0][0] != '/'
        : NULL == strchr(argv[0], '/')) return 0;

  #if 0
    /* disabled; not fully implemented
     * srv->srvconf.systemd_socket_activation might be cleared in network_init()
     * leading to issuing a false warning
     */
    /* warn if server.systemd-socket-activation not enabled
     * (While this warns on existing config rather than new config,
     *  it is probably a decent predictor for presence in new config) */
    if (!srv->srvconf.systemd_socket_activation)
        log_error(srv->errh, __FILE__, __LINE__,
          "[note] server.systemd-socket-activation not enabled; "
          "listen sockets will be closed and reopened");
  #endif

    /* flush log buffers to avoid potential duplication of entries
     * server_handle_sighup(srv) does the following, but skip logging */
    plugins_call_handle_sighup(srv);
    config_log_error_cycle(srv);

    /* backgrounding to continue processing requests in progress */
    /* re-exec lighttpd in original process
     *   Note: using path in re-exec is portable and allows lighttpd upgrade.
     *   OTOH, getauxval() AT_EXECFD and fexecve() could be used on Linux to
     *   re-exec without access to original executable on disk, which might be
     *   desirable in some situations, but is not implemented here.
     *   Alternatively, if argv[] was not available, could use readlink() on
     *   /proc/self/exe (Linux-specific), though there are ways on many other
     *   platforms to achieve the same:
     *   https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
     */
  #if defined(HAVE_KQUEUE)
   #if defined(__FreeBSD__) || defined(__DragonFly__)
    /*(must *exclude* rfork RFFDG flag for kqueue to work across rfork)*/
    pid_t pid = rfork(RFPROC);
   #else
    pid_t pid = -1;
    if (pid < 0) {
        /* kqueue is not inherited across fork
         * future: fdevent kqueue and stat_cache kqueue would need to be closed,
         *         re-opened, and active fds re-registered.  Not current done.
         *         Need to create some routines like fdevent_reinit_after_fork*/
        log_error(srv->errh, __FILE__, __LINE__,
          "server.graceful-restart-bg ignored on OpenBSD and NetBSD "
          "due to limitation in kqueue inheritance and lacking rfork");
        return 0;
    }
   #endif
  #else
    pid_t pid = fork();
  #endif
    if (pid) { /* original process */
        if (pid < 0) return 0;
        network_socket_activation_to_env(srv);
        /* save pid of original server in environment so that it can be
         * signalled by restarted server once restarted server is ready
         * to accept new connections */
        server_graceful_signal_prev_generation();/*(expect no prev gen active)*/
        if (0 == srv->srvconf.max_worker) {
            buffer * const tb = srv->tmp_buf;
            buffer_clear(tb);
            buffer_append_int(tb, pid);
            setenv("LIGHTTPD_PREV_GEN", tb->ptr, 1);
        }
        /*fdevent_waitpid(pid, NULL, 0);*//* detach? */
        execv(argv[0], argv);
        _exit(1);
    }
    /* else child/grandchild */

    /*if (-1 == setsid()) _exit(1);*//* should we detach? */
    /* Note: restarted server will fail with socket-in-use error if
     *       server.systemd-socket-activation not enabled in restarted server */
    if (0 != srv->srvconf.max_worker)
        server_sockets_close(srv);/*(close before parent reaps pid in waitpid)*/
    /*if (0 != fork())    _exit(0);*//* should we detach? */
    /*(grandchild is now backgrounded and detached from original process)*/

    /* XXX: might extend code to have new server.feature-flags param specify
     *      max lifetime before aborting remaining connections */

    /* (reached if lighttpd workers or if sole process w/o workers)
     * use same code as comment elsewhere in server.c:
     *   make sure workers do not muck with pid-file */
    if (0 <= pid_fd) {
            close(pid_fd);
            pid_fd = -1;
    }
    if (srv->srvconf.pid_file) buffer_clear(srv->srvconf.pid_file);

    /* (original process is backgrounded -- even if no active connections --
     *  to allow graceful shutdown tasks to be run by server and by modules) */
    log_error(srv->errh, __FILE__, __LINE__,
      "[note] pid %lld continuing to handle %u connection(s) in progress",
      (long long)getpid(), srv->conns.used);

    if (0 == srv->srvconf.max_worker) {
        /* reset graceful_shutdown; wait for signal from restarted server */
        srv->graceful_expire_ts = 0;
        graceful_shutdown = 0;
    }
    graceful_restart = 0;
    return 1;
}

__attribute_cold__
__attribute_noinline__
static void server_graceful_shutdown_maint (server *srv) {
    if (oneshot_fd) {
        /* permit keep-alive on one-shot connections until graceful_expire_ts */
        if (!srv->graceful_expire_ts) return;
        if (srv->graceful_expire_ts >= log_epoch_secs) return;
    }
    connection_graceful_shutdown_maint(srv);
}

__attribute_cold__
static void server_graceful_state (server *srv) {

    if (!srv_shutdown) {
        if (0 == srv->graceful_expire_ts && srv->srvconf.feature_flags) {
            const data_unset * const du =
              array_get_element_klen(srv->srvconf.feature_flags,
                CONST_STR_LEN("server.graceful-shutdown-timeout"));
            srv->graceful_expire_ts = config_plugin_value_to_int32(du, 0);
            if (srv->graceful_expire_ts)
                srv->graceful_expire_ts += log_epoch_secs;
        }
        server_graceful_shutdown_maint(srv);
    }

    if (2 == srv->sockets_disabled || 3 == srv->sockets_disabled) {
        if (oneshot_fd) graceful_restart = 0;
        return;
    }

    log_error(srv->errh,__FILE__,__LINE__,"[note] graceful shutdown started");

    /* no graceful restart if chroot()ed, if oneshot mode, or if idle timeout */
    if (!buffer_string_is_empty(srv->srvconf.changeroot)
        || oneshot_fd || 2 == graceful_shutdown)
        graceful_restart = 0;

    if (graceful_restart) {
        if (!server_graceful_state_bg(srv))
            server_sockets_unregister(srv);
        if (pid_fd > 0) pid_fd = -pid_fd; /*(flag to skip removing pid file)*/
    }
    else {
        server_sockets_close(srv);
        remove_pid_file(srv);
        /*(prevent more removal attempts)*/
        if (srv->srvconf.pid_file) buffer_clear(srv->srvconf.pid_file);
    }
}

__attribute_cold__
static void server_sockets_enable (server *srv) {
    server_sockets_set_event(srv, FDEVENT_IN);
    srv->sockets_disabled = 0;
    log_error(srv->errh, __FILE__, __LINE__, "[note] sockets enabled again");
}

__attribute_cold__
static void server_sockets_disable (server *srv) {
    server_sockets_set_event(srv, 0);
    srv->sockets_disabled = 1;
    log_error(srv->errh, __FILE__, __LINE__,
      (srv->conns.used >= srv->max_conns)
        ? "[note] sockets disabled, connection limit reached"
        : "[note] sockets disabled, out-of-fds");
}

__attribute_cold__
static void server_overload_check (server *srv) {
    if (srv->cur_fds + (int)srv->fdwaitqueue.used < srv->max_fds_lowat
        && srv->conns.used < srv->max_conns) {

        server_sockets_enable(srv);
    }
}

static void server_load_check (server *srv) {
    /* check if hit limits for num fds used or num connections */
    if (srv->cur_fds > srv->max_fds_hiwat || srv->conns.used >= srv->max_conns)
        server_sockets_disable(srv);
}

__attribute_cold__
__attribute_noinline__
static void server_process_fdwaitqueue (server *srv) {
    connections * const fdwaitqueue = &srv->fdwaitqueue;
    uint32_t i = 0;
    for (int n = srv->max_fds - srv->cur_fds - 16; n > 0; --n) {
        if (i == fdwaitqueue->used) break;
        connection_state_machine(fdwaitqueue->ptr[i++]);
    }
    if (i > 0 && 0 != (fdwaitqueue->used -= i)) {
	memmove(fdwaitqueue->ptr, fdwaitqueue->ptr+i, fdwaitqueue->used * sizeof(*(fdwaitqueue->ptr)));
    }
}

__attribute_cold__
static int server_main_setup (server * const srv, int argc, char **argv) {
	int print_config = 0;
	int test_config = 0;
	int i_am_root = 0;
	int o;
#ifdef HAVE_FORK
	int num_childs = 0;
#endif
	uint32_t i;
#ifdef HAVE_SIGACTION
	struct sigaction act;
#endif

#ifdef HAVE_FORK
	int parent_pipe_fd = -1;
#endif

#ifdef HAVE_GETUID
	i_am_root = (0 == getuid());
#endif

	/* initialize globals (including file-scoped static globals) */
	oneshot_fd = 0;
	oneshot_fdout = -1;
	srv_shutdown = 0;
	graceful_shutdown = 0;
	handle_sig_alarm = 1;
	handle_sig_hup = 0;
	idle_limit = 0;
	chunkqueue_set_tempdirs_default_reset();
	http_auth_dumbdata_reset();
	http_vhostdb_dumbdata_reset();
	/*graceful_restart = 0;*//*(reset below to avoid further daemonizing)*/
	/*(intentionally preserved)*/
	/*memset(graceful_sockets, 0, sizeof(graceful_sockets));*/
	/*memset(inherited_sockets, 0, sizeof(inherited_sockets));*/
	/*pid_fd = -1;*/
	srv->argv = argv;

	while(-1 != (o = getopt(argc, argv, "f:m:i:hvVD1pt"))) {
		switch(o) {
		case 'f':
			if (srv->config_data_base) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "Can only read one config file. Use the include command to use multiple config files.");
				return -1;
			}
			if (config_read(srv, optarg)) {
				return -1;
			}
			break;
		case 'm':
			buffer_copy_string(srv->srvconf.modules_dir, optarg);
			break;
		case 'i': {
			char *endptr;
			long timeout = strtol(optarg, &endptr, 0);
			if (!*optarg || *endptr || timeout < 0) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "Invalid idle timeout value: %s", optarg);
				return -1;
			}
			idle_limit = (time_t)timeout;
			break;
		}
		case 'p': print_config = 1; break;
		case 't': ++test_config; break;
		case '1': if (0 == oneshot_fd) oneshot_fd = dup(STDIN_FILENO);
			  break;
		case 'D': srv->srvconf.dont_daemonize = 1; break;
		case 'v': show_version(); return 0;
		case 'V': show_features(); return 0;
		case 'h': show_help(); return 0;
		default:
			show_help();
			return -1;
		}
	}

      #ifdef __CYGWIN__
	if (!srv->config_data_base && NULL != getenv("NSSM_SERVICE_NAME")) {
		char *dir = getenv("NSSM_SERVICE_DIR");
		if (NULL != dir && 0 != chdir(dir)) {
			log_perror(srv->errh, __FILE__, __LINE__, "chdir %s failed", dir);
			return -1;
		}
		srv->srvconf.dont_daemonize = 1;
		buffer_copy_string_len(srv->srvconf.modules_dir, CONST_STR_LEN("modules"));
		if (config_read(srv, "conf/lighttpd.conf")) return -1;
	}
      #endif

	if (!srv->config_data_base) {
		log_error(srv->errh, __FILE__, __LINE__,
		  "No configuration available. Try using -f option.");
		return -1;
	}

	if (print_config) {
		config_print(srv);
		fprintf(stdout, "\n");
	}

	if (test_config) {
		if (srv->srvconf.pid_file) buffer_clear(srv->srvconf.pid_file);
		if (1 == test_config) {
			printf("Syntax OK\n");
		} else { /*(test_config > 1)*/
			test_config = 0;
			srv->srvconf.preflight_check = 1;
			srv->srvconf.dont_daemonize = 1;
		}
	}

	if (test_config || print_config) {
		return 0;
	}

	if (oneshot_fd) {
		if (oneshot_fd <= STDERR_FILENO) {
			log_error(srv->errh, __FILE__, __LINE__,
			  "Invalid fds at startup with lighttpd -1");
			return -1;
		}
		graceful_shutdown = 1;
		srv->sockets_disabled = 2;
		srv->srvconf.dont_daemonize = 1;
		if (srv->srvconf.pid_file) buffer_clear(srv->srvconf.pid_file);
		if (srv->srvconf.max_worker) {
			srv->srvconf.max_worker = 0;
			log_error(srv->errh, __FILE__, __LINE__,
			  "server one-shot command line option disables server.max-worker config file option.");
		}

		struct stat st;
		if (0 != fstat(oneshot_fd, &st)) {
			log_perror(srv->errh, __FILE__, __LINE__, "fstat()");
			return -1;
		}

		if (S_ISFIFO(st.st_mode)) {
			oneshot_fdout = dup(STDOUT_FILENO);
			if (oneshot_fdout <= STDERR_FILENO) {
				log_perror(srv->errh, __FILE__, __LINE__, "dup()");
				return -1;
			}
		}
		else if (!S_ISSOCK(st.st_mode)) {
			/* require that fd is a socket
			 * (modules might expect STDIN_FILENO and STDOUT_FILENO opened to /dev/null) */
			log_error(srv->errh, __FILE__, __LINE__,
			  "lighttpd -1 stdin is not a socket");
			return -1;
		}
	}

	if (srv->srvconf.bindhost && buffer_is_equal_string(srv->srvconf.bindhost, CONST_STR_LEN("/dev/stdin"))) {
		if (-1 == srv->stdin_fd)
			srv->stdin_fd = dup(STDIN_FILENO);
		if (srv->stdin_fd <= STDERR_FILENO) {
			log_error(srv->errh, __FILE__, __LINE__,
			  "Invalid fds at startup");
			return -1;
		}
	}

	/* close stdin and stdout, as they are not needed */
	{
		struct stat st;
		int devnull;
		int errfd;
		do {
			/* coverity[overwrite_var : FALSE] */
			devnull = fdevent_open_devnull();
		      #ifdef __COVERITY__
			__coverity_escape__(devnull);
		      #endif
		} while (-1 != devnull && devnull <= STDERR_FILENO);
		if (-1 == devnull) {
			log_perror(srv->errh, __FILE__, __LINE__,
			  "opening /dev/null failed");
			return -1;
		}
		errfd = (0 == fstat(STDERR_FILENO, &st)) ? -1 : devnull;
		if (0 != fdevent_set_stdin_stdout_stderr(devnull, devnull, errfd)) {
			log_perror(srv->errh, __FILE__, __LINE__,
			  "setting default fds failed");
		      #ifdef FD_CLOEXEC
			if (-1 != errfd) close(errfd);
			if (devnull != errfd) close(devnull);
		      #endif
			return -1;
		}
	      #ifdef FD_CLOEXEC
		if (-1 != errfd) close(errfd);
		if (devnull != errfd) close(devnull);
	      #endif
	}

	http_response_send_1xx_cb_set(NULL, HTTP_VERSION_2);
	if (srv->srvconf.feature_flags
	    && !config_plugin_value_tobool(
	          array_get_element_klen(srv->srvconf.feature_flags,
	            CONST_STR_LEN("server.h2-discard-backend-1xx")), 0))
		http_response_send_1xx_cb_set(h2_send_1xx,
		                              HTTP_VERSION_2);

	http_response_send_1xx_cb_set(NULL, HTTP_VERSION_1_1);
	if (srv->srvconf.feature_flags
	    && !config_plugin_value_tobool(
	          array_get_element_klen(srv->srvconf.feature_flags,
	            CONST_STR_LEN("server.h1-discard-backend-1xx")), 0))
		http_response_send_1xx_cb_set(connection_send_1xx,
		                              HTTP_VERSION_1_1);

	if (0 != config_set_defaults(srv)) {
		log_error(srv->errh, __FILE__, __LINE__,
		  "setting default values failed");
		return -1;
	}

	if (plugins_load(srv)) {
		log_error(srv->errh, __FILE__, __LINE__,
		  "loading plugins finally failed");
		return -1;
	}

	if (HANDLER_GO_ON != plugins_call_init(srv)) {
		log_error(srv->errh, __FILE__, __LINE__,
		  "Initialization of plugins failed. Going down.");
		return -1;
	}

	/* mod_indexfile should be listed in server.modules prior to dynamic handlers */
	i = 0;
	for (const char *pname = NULL; i < srv->plugins.used; ++i) {
		plugin *p = ((plugin **)srv->plugins.ptr)[i];
		if (NULL != pname && 0 == strcmp(p->name, "indexfile")) {
			log_error(srv->errh, __FILE__, __LINE__,
			  "Warning: mod_indexfile should be listed in server.modules prior to mod_%s", pname);
			break;
		}
		if (p->handle_subrequest_start && p->handle_subrequest) {
			if (!pname) pname = p->name;
		}
	}

	/* open pid file BEFORE chroot */
	if (-2 == pid_fd) pid_fd = -1; /*(initial startup state)*/
	if (-1 == pid_fd && !buffer_string_is_empty(srv->srvconf.pid_file)) {
		const char *pidfile = srv->srvconf.pid_file->ptr;
		if (-1 == (pid_fd = fdevent_open_cloexec(pidfile, 0, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
			struct stat st;
			if (errno != EEXIST) {
				log_perror(srv->errh, __FILE__, __LINE__,
				  "opening pid-file failed: %s", pidfile);
				return -1;
			}

			if (0 != stat(pidfile, &st)) {
				log_perror(srv->errh, __FILE__, __LINE__,
				  "stating existing pid-file failed: %s", pidfile);
			}

			if (!S_ISREG(st.st_mode)) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "pid-file exists and isn't regular file: %s", pidfile);
				return -1;
			}

			if (-1 == (pid_fd = fdevent_open_cloexec(pidfile, 0, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
				log_perror(srv->errh, __FILE__, __LINE__,
				  "opening pid-file failed: %s", pidfile);
				return -1;
			}
		}
	}

	{
#ifdef HAVE_GETRLIMIT
		struct rlimit rlim = { 4096, 4096 };
		int use_rlimit = 1;
#ifdef HAVE_VALGRIND_VALGRIND_H
		if (RUNNING_ON_VALGRIND) use_rlimit = 0;
#endif

		if (0 != getrlimit(RLIMIT_NOFILE, &rlim)) {
			log_perror(srv->errh, __FILE__, __LINE__, "getrlimit()");
			use_rlimit = 0;
		}

		/**
		 * if we are not root can can't increase the fd-limit above rlim_max, but we can reduce it
		 */
		if (use_rlimit && srv->srvconf.max_fds
		    && (i_am_root || srv->srvconf.max_fds <= rlim.rlim_max)) {
			/* set rlimits */

			rlim.rlim_cur = srv->srvconf.max_fds;
			if (i_am_root) rlim.rlim_max = srv->srvconf.max_fds;

			if (0 != setrlimit(RLIMIT_NOFILE, &rlim)) {
				log_perror(srv->errh, __FILE__, __LINE__, "setrlimit()");
				return -1;
			}
		}

		/*(default upper limit of 4k if server.max-fds not specified)*/
		if (0 == srv->srvconf.max_fds)
			srv->srvconf.max_fds = (rlim.rlim_cur <= 4096)
			  ? (unsigned short)rlim.rlim_cur
			  : 4096;

		/* set core file rlimit, if enable_cores is set */
		if (use_rlimit && srv->srvconf.enable_cores && getrlimit(RLIMIT_CORE, &rlim) == 0) {
			rlim.rlim_cur = rlim.rlim_max;
			setrlimit(RLIMIT_CORE, &rlim);
		}
#endif
	}

	/* we need root-perms for port < 1024 */
	if (0 != network_init(srv, srv->stdin_fd)) {
		return -1;
	}
	srv->stdin_fd = -1;

	if (i_am_root) {
#ifdef HAVE_PWD_H
		/* set user and group */
		struct group *grp = NULL;
		struct passwd *pwd = NULL;

		if (!buffer_string_is_empty(srv->srvconf.groupname)) {
			if (NULL == (grp = getgrnam(srv->srvconf.groupname->ptr))) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "can't find groupname %s", srv->srvconf.groupname->ptr);
				return -1;
			}
		}

		if (!buffer_string_is_empty(srv->srvconf.username)) {
			if (NULL == (pwd = getpwnam(srv->srvconf.username->ptr))) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "can't find username %s", srv->srvconf.username->ptr);
				return -1;
			}

			if (pwd->pw_uid == 0) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "I will not set uid to 0\n");
				return -1;
			}

			if (NULL == grp && NULL == (grp = getgrgid(pwd->pw_gid))) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "can't find group id %d", (int)pwd->pw_gid);
				return -1;
			}
		}

		if (NULL != grp) {
			if (grp->gr_gid == 0) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "I will not set gid to 0\n");
				return -1;
			}
		}

		/* 
		 * Change group before chroot, when we have access
		 * to /etc/group
		 * */
		if (NULL != grp) {
			if (-1 == setgid(grp->gr_gid)) {
				log_perror(srv->errh, __FILE__, __LINE__, "setgid()");
				return -1;
			}
			if (-1 == setgroups(0, NULL)) {
				log_perror(srv->errh, __FILE__, __LINE__, "setgroups()");
				return -1;
			}
			if (!buffer_string_is_empty(srv->srvconf.username)) {
				initgroups(srv->srvconf.username->ptr, grp->gr_gid);
			}
		}
#endif
#ifdef HAVE_CHROOT
		if (!buffer_string_is_empty(srv->srvconf.changeroot)) {
			tzset();

			if (-1 == chroot(srv->srvconf.changeroot->ptr)) {
				log_perror(srv->errh, __FILE__, __LINE__, "chroot()");
				return -1;
			}
			if (-1 == chdir("/")) {
				log_perror(srv->errh, __FILE__, __LINE__, "chdir()");
				return -1;
			}
		}
#endif
#ifdef HAVE_PWD_H
		/* drop root privs */
		if (NULL != pwd) {
			if (-1 == setuid(pwd->pw_uid)) {
				log_perror(srv->errh, __FILE__, __LINE__, "setuid()");
				return -1;
			}
		}
#endif
#if defined(HAVE_SYS_PRCTL_H) && defined(PR_SET_DUMPABLE)
		/**
		 * on IRIX 6.5.30 they have prctl() but no DUMPABLE
		 */
		if (srv->srvconf.enable_cores) {
			prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
		}
#endif
	}

#ifdef HAVE_FORK
	/* network is up, let's daemonize ourself */
	if (0 == srv->srvconf.dont_daemonize && 0 == graceful_restart) {
		parent_pipe_fd = daemonize();
	}
#endif
	graceful_restart = 0;/*(reset here after avoiding further daemonizing)*/
	if (0 == oneshot_fd) graceful_shutdown = 0;


#ifdef HAVE_SIGACTION
	memset(&act, 0, sizeof(act));
	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &act, NULL);
# if defined(SA_SIGINFO)
	last_sighup_info.si_uid = 0,
	last_sighup_info.si_pid = 0;
	last_sigterm_info.si_uid = 0,
	last_sigterm_info.si_pid = 0;
	act.sa_sigaction = sigaction_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
# else
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
# endif
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP,  &act, NULL);
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);

	/* it should be safe to restart syscalls after SIGCHLD */
	act.sa_flags |= SA_RESTART | SA_NOCLDSTOP;
	sigaction(SIGCHLD, &act, NULL);

#elif defined(HAVE_SIGNAL)
	/* ignore the SIGPIPE from sendfile() */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP,  signal_handler);
	signal(SIGCHLD,  signal_handler);
	signal(SIGINT,  signal_handler);
	signal(SIGUSR1, signal_handler);
#endif


	srv->gid = getgid();
	srv->uid = getuid();
	srv->pid = getpid();

	/* write pid file */
	if (pid_fd > 2) {
		buffer * const tb = srv->tmp_buf;
		buffer_clear(tb);
		buffer_append_int(tb, srv->pid);
		buffer_append_string_len(tb, CONST_STR_LEN("\n"));
		if (-1 == write_all(pid_fd, CONST_BUF_LEN(tb))) {
			log_perror(srv->errh, __FILE__, __LINE__, "Couldn't write pid file");
			close(pid_fd);
			pid_fd = -1;
			return -1;
		}
	} else if (pid_fd < -2) {
		pid_fd = -pid_fd;
	}

	/* Close stderr ASAP in the child process to make sure that nothing
	 * is being written to that fd which may not be valid anymore. */
	if (!srv->srvconf.preflight_check) {
		if (-1 == config_log_error_open(srv)) {
			log_error(srv->errh, __FILE__, __LINE__, "Opening errorlog failed. Going down.");
			return -1;
		}
		if (!oneshot_fd)
			log_error(srv->errh, __FILE__, __LINE__, "server started (" PACKAGE_DESC ")");
	}

	if (HANDLER_GO_ON != plugins_call_set_defaults(srv)) {
		log_error(srv->errh, __FILE__, __LINE__, "Configuration of plugins failed. Going down.");
		return -1;
	}

	if (!config_finalize(srv, &default_server_tag)) {
		return -1;
	}

	if (srv->srvconf.preflight_check) {
		/*printf("Preflight OK");*//*(stdout reopened to /dev/null)*/
		return 0;
	}


#ifdef HAVE_FORK
	/**
	 * notify daemonize-grandparent of successful startup
	 * do this before any further forking is done (workers)
	 */
	if (0 == srv->srvconf.dont_daemonize && -1 != parent_pipe_fd) {
		if (0 > write(parent_pipe_fd, "", 1)) return -1;
		close(parent_pipe_fd);
	}

	if (idle_limit && srv->srvconf.max_worker) {
		srv->srvconf.max_worker = 0;
		log_error(srv->errh, __FILE__, __LINE__,
		  "server idle time limit command line option disables server.max-worker config file option.");
	}

	/* start watcher and workers */
	num_childs = srv->srvconf.max_worker;
	if (num_childs > 0) {
		pid_t pids[num_childs];
		pid_t pid;
		const int npids = num_childs;
		int child = 0;
		unsigned int timer = 0;
		for (int n = 0; n < npids; ++n) pids[n] = -1;
		server_graceful_signal_prev_generation();
		while (!child && !srv_shutdown && !graceful_shutdown) {
			if (num_childs > 0) {
				switch ((pid = fork())) {
				case -1:
					return -1;
				case 0:
					child = 1;
					alarm(0);
					break;
				default:
					num_childs--;
					for (int n = 0; n < npids; ++n) {
						if (-1 == pids[n]) {
							pids[n] = pid;
							break;
						}
					}
					break;
				}
			} else {
				int status;

				if (-1 != (pid = fdevent_waitpid(-1, &status, 0))) {
					log_epoch_secs = time(NULL);
					if (plugins_call_handle_waitpid(srv, pid, status) != HANDLER_GO_ON) {
						if (!timer) alarm((timer = 5));
						continue;
					}
					switch (fdevent_reaped_logger_pipe(pid)) {
					  default: break;
					  case -1: if (!timer) alarm((timer = 5));
						   __attribute_fallthrough__
					  case  1: continue;
					}
					/** 
					 * check if one of our workers went away
					 */
					for (int n = 0; n < npids; ++n) {
						if (pid == pids[n]) {
							pids[n] = -1;
							num_childs++;
							break;
						}
					}
				} else {
					switch (errno) {
					case EINTR:
						log_epoch_secs = time(NULL);
						/**
						 * if we receive a SIGHUP we have to close our logs ourself as we don't 
						 * have the mainloop who can help us here
						 */
						if (handle_sig_hup) {
							handle_sig_hup = 0;

							config_log_error_cycle(srv);

							/* forward SIGHUP to workers */
							for (int n = 0; n < npids; ++n) {
								if (pids[n] > 0) kill(pids[n], SIGHUP);
							}
						}
						if (handle_sig_alarm) {
							handle_sig_alarm = 0;
							timer = 0;
							plugins_call_handle_trigger(srv);
							fdevent_restart_logger_pipes(log_epoch_secs);
						}
						break;
					default:
						break;
					}
				}
			}
		}

		/**
		 * for the parent this is the exit-point 
		 */
		if (!child) {
			/** 
			 * kill all children too 
			 */
			if (graceful_shutdown || graceful_restart) {
				/* flag to ignore one SIGINT if graceful_restart */
				if (graceful_restart) graceful_restart = 2;
				kill(0, SIGINT);
				server_graceful_state(srv);
			} else if (srv_shutdown) {
				kill(0, SIGTERM);
			}

			return 0;
		}

		/* ignore SIGUSR1 in workers; only parent directs graceful restart */
	      #ifdef HAVE_SIGACTION
		{
			struct sigaction actignore;
			memset(&actignore, 0, sizeof(actignore));
			actignore.sa_handler = SIG_IGN;
			sigaction(SIGUSR1, &actignore, NULL);
		}
	      #elif defined(HAVE_SIGNAL)
			signal(SIGUSR1, SIG_IGN);
	      #endif

		/**
		 * make sure workers do not muck with pid-file
		 */
		if (0 <= pid_fd) {
			close(pid_fd);
			pid_fd = -1;
		}
		if (srv->srvconf.pid_file) buffer_clear(srv->srvconf.pid_file);

		fdevent_clr_logger_pipe_pids();
		srv->pid = getpid();
		li_rand_reseed();
	}
#endif

	srv->max_fds = (int)srv->srvconf.max_fds;
	srv->ev = fdevent_init(srv->srvconf.event_handler, &srv->max_fds, &srv->cur_fds, srv->errh);
	if (NULL == srv->ev) {
		log_error(srv->errh, __FILE__, __LINE__, "fdevent_init failed");
		return -1;
	}

	srv->max_fds_lowat = srv->max_fds * 8 / 10;
	srv->max_fds_hiwat = srv->max_fds * 9 / 10;

	/* set max-conns */
	if (srv->srvconf.max_conns > srv->max_fds/2) {
		/* we can't have more connections than max-fds/2 */
		log_error(srv->errh, __FILE__, __LINE__,
		  "can't have more connections than fds/2: %hu %d",
		  srv->srvconf.max_conns, srv->max_fds);
		srv->max_conns = srv->max_fds/2;
	} else if (srv->srvconf.max_conns) {
		/* otherwise respect the wishes of the user */
		srv->max_conns = srv->srvconf.max_conns;
	} else {
		/* or use the default: we really don't want to hit max-fds */
		srv->max_conns = srv->max_fds/3;
	}

	request_pool_init(srv->max_conns);

	/* libev backend overwrites our SIGCHLD handler and calls waitpid on SIGCHLD; we want our own SIGCHLD handling. */
#ifdef HAVE_SIGACTION
	sigaction(SIGCHLD, &act, NULL);
#elif defined(HAVE_SIGNAL)
	signal(SIGCHLD,  signal_handler);
#endif

	/*
	 * kqueue() is called here, select resets its internals,
	 * all server sockets get their handlers
	 *
	 * */
	if (0 != network_register_fdevents(srv)) {
		return -1;
	}

	/* might fail if user is using fam (not gamin) and famd isn't running */
	if (!stat_cache_init(srv->ev, srv->errh)) {
		log_error(srv->errh, __FILE__, __LINE__,
		  "stat-cache could not be setup, dying.");
		return -1;
	}

#ifdef USE_ALARM
	{
		/* setup periodic timer (1 second) */
		struct itimerval interval;
		interval.it_interval.tv_sec = 1;
		interval.it_interval.tv_usec = 0;
		interval.it_value.tv_sec = 1;
		interval.it_value.tv_usec = 0;
		if (setitimer(ITIMER_REAL, &interval, NULL)) {
			log_perror(srv->errh, __FILE__, __LINE__, "setitimer()");
			return -1;
		}
	}
#endif

	/* get the current number of FDs */
	{
		int fd = fdevent_open_devnull();
		if (fd >= 0) {
			srv->cur_fds = fd;
			close(fd);
		}
	}

	if (0 != server_sockets_set_nb_cloexec(srv)) {
		return -1;
	}

	/* plugin hook for worker_init */
	if (HANDLER_GO_ON != plugins_call_worker_init(srv))
		return -1;

	if (oneshot_fdout > 0) {
		if (server_oneshot_init_pipe(srv, oneshot_fd, oneshot_fdout)) {
			oneshot_fd = -1;
			oneshot_fdout = -1;
		}
	}
	else if (oneshot_fd && server_oneshot_init(srv, oneshot_fd)) {
		oneshot_fd = -1;
	}

	if (0 == srv->srvconf.max_worker)
		server_graceful_signal_prev_generation();

	return 1;
}

__attribute_cold__
__attribute_noinline__
static void server_handle_sighup (server * const srv) {

			/* cycle logfiles */

			plugins_call_handle_sighup(srv);

			config_log_error_cycle(srv);
#ifdef HAVE_SIGACTION
				log_error(srv->errh, __FILE__, __LINE__,
				  "logfiles cycled UID = %d PID = %d",
				  (int)last_sighup_info.si_uid,
				  (int)last_sighup_info.si_pid);
#else
				log_error(srv->errh, __FILE__, __LINE__,
				  "logfiles cycled");
#endif
}

__attribute_noinline__
static void server_handle_sigalrm (server * const srv, time_t min_ts, time_t last_active_ts) {

				plugins_call_handle_trigger(srv);

				log_epoch_secs = min_ts;

				/* check idle time limit, if enabled */
				if (idle_limit && idle_limit < min_ts - last_active_ts && !graceful_shutdown) {
					log_error(srv->errh, __FILE__, __LINE__,
					  "[note] idle timeout %ds exceeded, "
					  "initiating graceful shutdown", (int)idle_limit);
					graceful_shutdown = 2; /* value 2 indicates idle timeout */
					if (graceful_restart) {
						graceful_restart = 0;
						if (pid_fd < -2) pid_fd = -pid_fd;
						server_sockets_close(srv);
					}
				}

			      #ifdef HAVE_GETLOADAVG
				/* refresh loadavg data every 30 seconds */
				if (srv->loadts + 30 < min_ts) {
					if (-1 != getloadavg(srv->loadavg, 3)) {
						srv->loadts = min_ts;
					}
				}
			      #endif

				if (0 == (min_ts & 0x3f)) { /*(once every 64 secs)*/
					/* free excess chunkqueue buffers every 64 secs */
					chunkqueue_chunk_pool_clear();
					/* attempt to restart dead piped loggers every 64 secs */
					if (0 == srv->srvconf.max_worker)
						fdevent_restart_logger_pipes(min_ts);
				}
				/* cleanup stat-cache */
				stat_cache_trigger_cleanup();
				/* reset global/aggregate rate limit counters */
				config_reset_config_bytes_sec(srv->config_data_base);
				/* if graceful_shutdown, accelerate cleanup of recently completed request/responses */
				if (graceful_shutdown && !srv_shutdown)
					server_graceful_shutdown_maint(srv);
				connection_periodic_maint(srv, min_ts);
}

__attribute_noinline__
static void server_handle_sigchld (server * const srv) {
			pid_t pid;
			do {
				int status;
				pid = fdevent_waitpid(-1, &status, 1);
				if (pid > 0) {
					if (plugins_call_handle_waitpid(srv, pid, status) != HANDLER_GO_ON) {
						continue;
					}
					if (0 == srv->srvconf.max_worker) {
						/* check piped-loggers and restart, even if shutting down */
						if (fdevent_waitpid_logger_pipe_pid(pid, log_epoch_secs)) {
							continue;
						}
					}
				}
			} while (pid > 0 || (-1 == pid && errno == EINTR));
}

__attribute_hot__
static void server_run_con_queue (connections * const restrict joblist) {
    connection * const * const restrict conlist = joblist->ptr;
    const uint32_t used = joblist->used;
    joblist->used = 0;
    for (uint32_t i = 0; i < used; ++i) {
        connection_state_machine(conlist[i]);
    }
}

__attribute_hot__
__attribute_noinline__
static void server_main_loop (server * const srv) {
	time_t last_active_ts = time(NULL);

	while (!srv_shutdown) {

		if (handle_sig_hup) {
			handle_sig_hup = 0;
			server_handle_sighup(srv);
		}

		/*(USE_ALARM not used; fdevent_poll() is effective periodic timer)*/
	      #ifdef USE_ALARM
		if (handle_sig_alarm) {
			handle_sig_alarm = 0;
	      #endif
			time_t min_ts = time(NULL);
			if (min_ts != log_epoch_secs) {
				server_handle_sigalrm(srv, min_ts, last_active_ts);
			}
	      #ifdef USE_ALARM
		}
	      #endif

		if (handle_sig_child) {
			handle_sig_child = 0;
			server_handle_sigchld(srv);
		}

		if (graceful_shutdown) {
			server_graceful_state(srv);
			if (0 == srv->conns.used && graceful_shutdown) {
				/* we are in graceful shutdown phase and all connections are closed
				 * we are ready to terminate without harming anyone */
				srv_shutdown = 1;
				break;
			}
		} else if (srv->sockets_disabled) {
			server_overload_check(srv);
		} else {
			server_load_check(srv);
		}

		if (srv->fdwaitqueue.used) {
			server_process_fdwaitqueue(srv);
		}

		connections * const joblist = connection_joblist;

		if (fdevent_poll(srv->ev, joblist->used ? 0 : 1000) > 0) {
			last_active_ts = log_epoch_secs;
		}

		connection_joblist = (joblist == &srv->joblist_A)
		  ? &srv->joblist_B
		  : &srv->joblist_A;

		server_run_con_queue(joblist);
	}
}

__attribute_cold__
int main (int argc, char **argv) {
    int rc;

  #ifdef HAVE_GETUID
  #ifndef HAVE_ISSETUGID
  #define issetugid() (geteuid() != getuid() || getegid() != getgid())
  #endif
    if (0 != getuid() && issetugid()) { /*check as early as possible in main()*/
        fprintf(stderr,
                "Are you nuts ? Don't apply a SUID bit to this binary\n");
        return -1;
    }
  #endif

    /* for nice %b handling in strftime() */
    setlocale(LC_TIME, "C");
    tzset();

    do {
        server * const srv = server_init();

        if (graceful_restart) {
            server_sockets_restore(srv);
            optind = 1;
        }

        rc = server_main_setup(srv, argc, argv);
        if (rc > 0) {

            server_main_loop(srv);

            if (graceful_shutdown || graceful_restart) {
                server_graceful_state(srv);
            }

            if (2 == graceful_shutdown) { /* value 2 indicates idle timeout */
                log_error(srv->errh, __FILE__, __LINE__,
                  "server stopped after idle timeout");
            } else if (!oneshot_fd) {
              #ifdef HAVE_SIGACTION
                log_error(srv->errh, __FILE__, __LINE__,
                  "server stopped by UID = %d PID = %d",
                  (int)last_sigterm_info.si_uid,
                  (int)last_sigterm_info.si_pid);
              #else
                log_error(srv->errh, __FILE__, __LINE__,
                  "server stopped");
              #endif
            }
        }

        /* clean-up */
        remove_pid_file(srv);
        config_log_error_close(srv);
        if (graceful_restart)
            server_sockets_save(srv);
        else
            network_close(srv);
        request_pool_free();
        connections_free(srv);
        plugins_free(srv);
        server_free(srv);

        if (rc < 0 || !graceful_restart) break;

        /* wait for all children to exit before graceful restart */
        while (fdevent_waitpid(-1, NULL, 0) > 0) ;
    } while (graceful_restart);

    return rc;
}
