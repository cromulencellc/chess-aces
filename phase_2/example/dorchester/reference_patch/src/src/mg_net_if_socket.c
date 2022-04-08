
#if MG_ENABLE_NET_IF_SOCKET

#include "mg_net_if_socket.h"
#include "mg_internal.h"
#include "mg_util.h"

static sock_t mg_open_listening_socket(union socket_address *sa, int type,
                                       int proto);

void mg_set_non_blocking_mode(sock_t sock) {
#ifdef _WIN32
  unsigned long on = 1;
  ioctlsocket(sock, FIONBIO, &on);
#else
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

static int mg_is_error(void) {
  int err = mg_get_errno();
  return err != EINPROGRESS && err != EWOULDBLOCK
#ifndef WINCE
         && err != EAGAIN && err != EINTR
#endif
#ifdef _WIN32
         && WSAGetLastError() != WSAEINTR && WSAGetLastError() != WSAEWOULDBLOCK
#endif
      ;
}

void mg_socket_if_connect_tcp(struct mg_connection *nc,
                              const union socket_address *sa) {
  int rc, proto = 0;
  nc->sock = socket(AF_INET, SOCK_STREAM, proto);
  if (nc->sock == INVALID_SOCKET) {
    nc->err = mg_get_errno() ? mg_get_errno() : 1;
    return;
  }
#if !defined(MG_ESP8266)
  mg_set_non_blocking_mode(nc->sock);
#endif
  rc = connect(nc->sock, &sa->sa, sizeof(sa->sin));
  nc->err = rc < 0 && mg_is_error() ? mg_get_errno() : 0;
  DBG(("%p sock %d rc %d errno %d err %d", nc, (int) nc->sock, rc,
      mg_get_errno(), nc->err));
}

void mg_socket_if_connect_udp(struct mg_connection *nc) {
  nc->sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (nc->sock == INVALID_SOCKET) {
    nc->err = mg_get_errno() ? mg_get_errno() : 1;
    return;
  }
  if (nc->flags & MG_F_ENABLE_BROADCAST) {
    int optval = 1;
    if (setsockopt(nc->sock, SOL_SOCKET, SO_BROADCAST, (const char *) &optval,
                   sizeof(optval)) < 0) {
      nc->err = mg_get_errno() ? mg_get_errno() : 1;
      return;
    }
  }
  nc->err = 0;
}

int mg_socket_if_listen_tcp(struct mg_connection *nc,
                            union socket_address *sa) {
  int proto = 0;
  sock_t sock = mg_open_listening_socket(sa, SOCK_STREAM, proto);
  if (sock == INVALID_SOCKET) {
    return (mg_get_errno() ? mg_get_errno() : 1);
  }
  mg_sock_set(nc, sock);
  return 0;
}

static int mg_socket_if_listen_udp(struct mg_connection *nc,
                                   union socket_address *sa) {
  sock_t sock = mg_open_listening_socket(sa, SOCK_DGRAM, 0);
  if (sock == INVALID_SOCKET) return (mg_get_errno() ? mg_get_errno() : 1);
  mg_sock_set(nc, sock);
  return 0;
}

static int mg_socket_if_tcp_send(struct mg_connection *nc, const void *buf,
                                 size_t len) {
  int n = (int) MG_SEND_FUNC(nc->sock, buf, len, 0);
  if (n < 0 && !mg_is_error()) n = 0;
  return n;
}

static int mg_socket_if_udp_send(struct mg_connection *nc, const void *buf,
                                 size_t len) {
  int n = sendto(nc->sock, buf, len, 0, &nc->sa.sa, sizeof(nc->sa.sin));
  if (n < 0 && !mg_is_error()) n = 0;
  return n;
}

static int mg_socket_if_tcp_recv(struct mg_connection *nc, void *buf,
                                 size_t len) {
  int n = (int) MG_RECV_FUNC(nc->sock, buf, len, 0);
  if (n == 0) {
    /* Orderly shutdown of the socket, try flushing output. */
    nc->flags |= MG_F_SEND_AND_CLOSE;
  } else if (n < 0 && !mg_is_error()) {
    n = 0;
  }
  return n;
}

static int mg_socket_if_udp_recv(struct mg_connection *nc, void *buf,
                                 size_t len, union socket_address *sa,
                                 size_t *sa_len) {
  socklen_t sa_len_st = *sa_len;
  int n = recvfrom(nc->sock, buf, len, 0, &sa->sa, &sa_len_st);
  *sa_len = sa_len_st;
  if (n < 0 && !mg_is_error()) n = 0;
  return n;
}

int mg_socket_if_create_conn(struct mg_connection *nc) {
  (void) nc;
  return 1;
}

void mg_socket_if_destroy_conn(struct mg_connection *nc) {
  if (nc->sock == INVALID_SOCKET) return;
  if (!(nc->flags & MG_F_UDP)) {
    closesocket(nc->sock);
  } else {
    /* Only close outgoing UDP sockets or listeners. */
    if (nc->listener == NULL) closesocket(nc->sock);
  }
  nc->sock = INVALID_SOCKET;
}

static int mg_accept_conn(struct mg_connection *lc) {
  struct mg_connection *nc;
  union socket_address sa;
  socklen_t sa_len = sizeof(sa);
  /* NOTE(lsm): on Windows, sock is always > FD_SETSIZE */
  sock_t sock = accept(lc->sock, &sa.sa, &sa_len);
  if (sock == INVALID_SOCKET) {
    if (mg_is_error()) {
      DBG(("%p: failed to accept: %d", lc, mg_get_errno()));
    }
    return 0;
  }
  nc = mg_if_accept_new_conn(lc);
  if (nc == NULL) {
    closesocket(sock);
    return 0;
  }
  DBG(("%p conn from %s:%d", nc, inet_ntoa(sa.sin.sin_addr),
       ntohs(sa.sin.sin_port)));
  mg_sock_set(nc, sock);
  mg_if_accept_tcp_cb(nc, &sa, sa_len);
  return 1;
}

/* 'sa' must be an initialized address to bind to */
static sock_t mg_open_listening_socket(union socket_address *sa, int type,
                                       int proto) {
  socklen_t sa_len =
      (sa->sa.sa_family == AF_INET) ? sizeof(sa->sin) : sizeof(sa->sin6);
  sock_t sock = INVALID_SOCKET;
#if !MG_LWIP
  int on = 1;
#endif

  if ((sock = socket(sa->sa.sa_family, type, proto)) != INVALID_SOCKET &&
#if !MG_LWIP /* LWIP doesn't support either */
#if defined(_WIN32) && defined(SO_EXCLUSIVEADDRUSE) && !defined(WINCE)
      /* "Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE" http://goo.gl/RmrFTm */
      !setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (void *) &on,
                  sizeof(on)) &&
#endif

#if !defined(_WIN32) || !defined(SO_EXCLUSIVEADDRUSE)
      /*
       * SO_RESUSEADDR is not enabled on Windows because the semantics of
       * SO_REUSEADDR on UNIX and Windows is different. On Windows,
       * SO_REUSEADDR allows to bind a socket to a port without error even if
       * the port is already open by another program. This is not the behavior
       * SO_REUSEADDR was designed for, and leads to hard-to-track failure
       * scenarios. Therefore, SO_REUSEADDR was disabled on Windows unless
       * SO_EXCLUSIVEADDRUSE is supported and set on a socket.
       */
      !setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on)) &&
#endif
#endif /* !MG_LWIP */

      !bind(sock, &sa->sa, sa_len) &&
      (type == SOCK_DGRAM || listen(sock, SOMAXCONN) == 0)) {
#if !MG_LWIP
    mg_set_non_blocking_mode(sock);
    /* In case port was set to 0, get the real port number */
    (void) getsockname(sock, &sa->sa, &sa_len);
#endif
  } else if (sock != INVALID_SOCKET) {
    closesocket(sock);
    sock = INVALID_SOCKET;
  }

  return sock;
}

#define _MG_F_FD_CAN_READ 1
#define _MG_F_FD_CAN_WRITE 1 << 1
#define _MG_F_FD_ERROR 1 << 2

void mg_mgr_handle_conn(struct mg_connection *nc, int fd_flags, double now) {
  int worth_logging =
      fd_flags != 0 || (nc->flags & (MG_F_WANT_READ | MG_F_WANT_WRITE));
  if (worth_logging) {
    DBG(("%p fd=%d fd_flags=%d nc_flags=0x%lx rmbl=%d smbl=%d", nc,
         (int) nc->sock, fd_flags, nc->flags, (int) nc->recv_mbuf.len,
         (int) nc->send_mbuf.len));
  }

  if (!mg_if_poll(nc, now)) return;

  if (nc->flags & MG_F_CONNECTING) {
    if (fd_flags != 0) {
      int err = 0;
#if !defined(MG_ESP8266)
      if (!(nc->flags & MG_F_UDP)) {
        socklen_t len = sizeof(err);
        int ret =
            getsockopt(nc->sock, SOL_SOCKET, SO_ERROR, (char *) &err, &len);
        if (ret != 0) {
          err = 1;
        } else if (err == EAGAIN || err == EWOULDBLOCK) {
          err = 0;
        }
      }
#else
      /*
       * On ESP8266 we use blocking connect.
       */
      err = nc->err;
#endif
      mg_if_connect_cb(nc, err);
    } else if (nc->err != 0) {
      mg_if_connect_cb(nc, nc->err);
    }
  }

  if (fd_flags & _MG_F_FD_CAN_READ) {
    if (nc->flags & MG_F_UDP) {
      mg_if_can_recv_cb(nc);
    } else {
      if (nc->flags & MG_F_LISTENING) {
        /*
         * We're not looping here, and accepting just one connection at
         * a time. The reason is that eCos does not respect non-blocking
         * flag on a listening socket and hangs in a loop.
         */
        mg_accept_conn(nc);
      } else {
        mg_if_can_recv_cb(nc);
      }
    }
  }

  if (fd_flags & _MG_F_FD_CAN_WRITE) mg_if_can_send_cb(nc);

  if (worth_logging) {
    DBG(("%p after fd=%d nc_flags=0x%lx rmbl=%d smbl=%d", nc, (int) nc->sock,
         nc->flags, (int) nc->recv_mbuf.len, (int) nc->send_mbuf.len));
  }
}

#if MG_ENABLE_BROADCAST
static void mg_mgr_handle_ctl_sock(struct mg_mgr *mgr) {
  struct ctl_msg ctl_msg;
  int len = (int) MG_RECV_FUNC(mgr->ctl[1], (char *) &ctl_msg, sizeof(ctl_msg), 0);
  size_t dummy = MG_SEND_FUNC(mgr->ctl[1], ctl_msg.message, 1, 0);
  DBG(("read %d from ctl socket", len));
  (void) dummy; /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509 */
  if (len >= (int) sizeof(ctl_msg.callback) && ctl_msg.callback != NULL) {
    struct mg_connection *nc;
    for (nc = mg_next(mgr, NULL); nc != NULL; nc = mg_next(mgr, nc)) {
      ctl_msg.callback(nc, MG_EV_POLL,
                       ctl_msg.message MG_UD_ARG(nc->user_data));
    }
  }
}
#endif

/* Associate a socket to a connection. */
void mg_socket_if_sock_set(struct mg_connection *nc, sock_t sock) {
  mg_set_non_blocking_mode(sock);
  mg_set_close_on_exec(sock);
  nc->sock = sock;
  DBG(("%p %d", nc, (int) sock));
}

void mg_socket_if_init(struct mg_iface *iface) {
  (void) iface;
  DBG(("%p using select()", iface->mgr));
#if MG_ENABLE_BROADCAST
  mg_socketpair(iface->mgr->ctl, SOCK_DGRAM);
#endif
}

void mg_socket_if_free(struct mg_iface *iface) {
  (void) iface;
}

void mg_socket_if_add_conn(struct mg_connection *nc) {
  (void) nc;
}

void mg_socket_if_remove_conn(struct mg_connection *nc) {
  (void) nc;
}

void mg_add_to_set(sock_t sock, fd_set *set, sock_t *max_fd) {
  if (sock != INVALID_SOCKET
#ifdef __unix__
      && sock < (sock_t) FD_SETSIZE
#endif
      ) {
    FD_SET(sock, set);
    if (*max_fd == INVALID_SOCKET || sock > *max_fd) {
      *max_fd = sock;
    }
  }
}

time_t mg_socket_if_poll(struct mg_iface *iface, int timeout_ms) {
  struct mg_mgr *mgr = iface->mgr;
  double now = mg_time();
  double min_timer;
  struct mg_connection *nc, *tmp;
  struct timeval tv;
  fd_set read_set, write_set, err_set;
  sock_t max_fd = INVALID_SOCKET;
  int num_fds, num_ev, num_timers = 0;
#ifdef __unix__
  int try_dup = 1;
#endif

  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_ZERO(&err_set);
#if MG_ENABLE_BROADCAST
  mg_add_to_set(mgr->ctl[1], &read_set, &max_fd);
#endif

  /*
   * Note: it is ok to have connections with sock == INVALID_SOCKET in the list,
   * e.g. timer-only "connections".
   */
  min_timer = 0;
  for (nc = mgr->active_connections, num_fds = 0; nc != NULL; nc = tmp) {
    tmp = nc->next;

    if (nc->sock != INVALID_SOCKET) {
      num_fds++;

#ifdef __unix__
      /* A hack to make sure all our file descriptos fit into FD_SETSIZE. */
      if (nc->sock >= (sock_t) FD_SETSIZE && try_dup) {
        int new_sock = dup(nc->sock);
        if (new_sock >= 0) {
          if (new_sock < (sock_t) FD_SETSIZE) {
            closesocket(nc->sock);
            DBG(("new sock %d -> %d", nc->sock, new_sock));
            nc->sock = new_sock;
          } else {
            closesocket(new_sock);
            DBG(("new sock is still larger than FD_SETSIZE, disregard"));
            try_dup = 0;
          }
        } else {
          try_dup = 0;
        }
      }
#endif

      if (nc->recv_mbuf.len < nc->recv_mbuf_limit &&
          (!(nc->flags & MG_F_UDP) || nc->listener == NULL)) {
        mg_add_to_set(nc->sock, &read_set, &max_fd);
      }

      if (((nc->flags & MG_F_CONNECTING) && !(nc->flags & MG_F_WANT_READ)) ||
          (nc->send_mbuf.len > 0 && !(nc->flags & MG_F_CONNECTING))) {
        mg_add_to_set(nc->sock, &write_set, &max_fd);
        mg_add_to_set(nc->sock, &err_set, &max_fd);
      }
    }

    if (nc->ev_timer_time > 0) {
      if (num_timers == 0 || nc->ev_timer_time < min_timer) {
        min_timer = nc->ev_timer_time;
      }
      num_timers++;
    }
  }

  /*
   * If there is a timer to be fired earlier than the requested timeout,
   * adjust the timeout.
   */
  if (num_timers > 0) {
    double timer_timeout_ms = (min_timer - mg_time()) * 1000 + 1 /* rounding */;
    if (timer_timeout_ms < timeout_ms) {
      timeout_ms = (int) timer_timeout_ms;
    }
  }
  if (timeout_ms < 0) timeout_ms = 0;

  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  num_ev = select((int) max_fd + 1, &read_set, &write_set, &err_set, &tv);
  now = mg_time();
#if 0
  DBG(("select @ %ld num_ev=%d of %d, timeout=%d", (long) now, num_ev, num_fds,
       timeout_ms));
#endif

#if MG_ENABLE_BROADCAST
  if (num_ev > 0 && mgr->ctl[1] != INVALID_SOCKET &&
      FD_ISSET(mgr->ctl[1], &read_set)) {
    mg_mgr_handle_ctl_sock(mgr);
  }
#endif

  for (nc = mgr->active_connections; nc != NULL; nc = tmp) {
    int fd_flags = 0;
    if (nc->sock != INVALID_SOCKET) {
      if (num_ev > 0) {
        fd_flags = (FD_ISSET(nc->sock, &read_set) &&
                            (!(nc->flags & MG_F_UDP) || nc->listener == NULL)
                        ? _MG_F_FD_CAN_READ
                        : 0) |
                   (FD_ISSET(nc->sock, &write_set) ? _MG_F_FD_CAN_WRITE : 0) |
                   (FD_ISSET(nc->sock, &err_set) ? _MG_F_FD_ERROR : 0);
      }
#if MG_LWIP
      /* With LWIP socket emulation layer, we don't get write events for UDP */
      if ((nc->flags & MG_F_UDP) && nc->listener == NULL) {
        fd_flags |= _MG_F_FD_CAN_WRITE;
      }
#endif
    }
    tmp = nc->next;
    mg_mgr_handle_conn(nc, fd_flags, now);
  }

  return (time_t) now;
}

#if MG_ENABLE_BROADCAST
MG_INTERNAL void mg_socketpair_close(sock_t *sock) {
  while (1) {
    if (closesocket(*sock) == -1 && errno == EINTR) continue;
    break;
  }
  *sock = INVALID_SOCKET;
}

MG_INTERNAL sock_t
mg_socketpair_accept(sock_t sock, union socket_address *sa, socklen_t sa_len) {
  sock_t rc;
  while (1) {
    if ((rc = accept(sock, &sa->sa, &sa_len)) == INVALID_SOCKET &&
        errno == EINTR)
      continue;
    break;
  }
  return rc;
}

int mg_socketpair(sock_t sp[2], int sock_type) {
  union socket_address sa, sa2;
  sock_t sock;
  socklen_t len = sizeof(sa.sin);
  int ret = 0;

  sock = sp[0] = sp[1] = INVALID_SOCKET;

  (void) memset(&sa, 0, sizeof(sa));
  sa.sin.sin_family = AF_INET;
  sa.sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
  sa2 = sa;

  if ((sock = socket(AF_INET, sock_type, 0)) == INVALID_SOCKET) {
  } else if (bind(sock, &sa.sa, len) != 0) {
  } else if (sock_type == SOCK_STREAM && listen(sock, 1) != 0) {
  } else if (getsockname(sock, &sa.sa, &len) != 0) {
  } else if ((sp[0] = socket(AF_INET, sock_type, 0)) == INVALID_SOCKET) {
  } else if (sock_type == SOCK_STREAM && connect(sp[0], &sa.sa, len) != 0) {
  } else if (sock_type == SOCK_DGRAM &&
             (bind(sp[0], &sa2.sa, len) != 0 ||
              getsockname(sp[0], &sa2.sa, &len) != 0 ||
              connect(sp[0], &sa.sa, len) != 0 ||
              connect(sock, &sa2.sa, len) != 0)) {
  } else if ((sp[1] = (sock_type == SOCK_DGRAM ? sock : mg_socketpair_accept(
                                                            sock, &sa, len))) ==
             INVALID_SOCKET) {
  } else {
    mg_set_close_on_exec(sp[0]);
    mg_set_close_on_exec(sp[1]);
    if (sock_type == SOCK_STREAM) mg_socketpair_close(&sock);
    ret = 1;
  }

  if (!ret) {
    if (sp[0] != INVALID_SOCKET) mg_socketpair_close(&sp[0]);
    if (sp[1] != INVALID_SOCKET) mg_socketpair_close(&sp[1]);
    if (sock != INVALID_SOCKET) mg_socketpair_close(&sock);
  }

  return ret;
}
#endif /* MG_ENABLE_BROADCAST */

static void mg_sock_get_addr(sock_t sock, int remote,
                             union socket_address *sa) {
  socklen_t slen = sizeof(*sa);
  memset(sa, 0, slen);
  if (remote) {
    getpeername(sock, &sa->sa, &slen);
  } else {
    getsockname(sock, &sa->sa, &slen);
  }
}

void mg_sock_to_str(sock_t sock, char *buf, size_t len, int flags) {
  union socket_address sa;
  mg_sock_get_addr(sock, flags & MG_SOCK_STRINGIFY_REMOTE, &sa);
  mg_sock_addr_to_str(&sa, buf, len, flags);
}

void mg_socket_if_get_conn_addr(struct mg_connection *nc, int remote,
                                union socket_address *sa) {
  if ((nc->flags & MG_F_UDP) && remote) {
    memcpy(sa, &nc->sa, sizeof(*sa));
    return;
  }
  mg_sock_get_addr(nc->sock, remote, sa);
}

/* clang-format off */
#define MG_SOCKET_IFACE_VTABLE                                          \
  {                                                                     \
    mg_socket_if_init,                                                  \
    mg_socket_if_free,                                                  \
    mg_socket_if_add_conn,                                              \
    mg_socket_if_remove_conn,                                           \
    mg_socket_if_poll,                                                  \
    mg_socket_if_listen_tcp,                                            \
    mg_socket_if_listen_udp,                                            \
    mg_socket_if_connect_tcp,                                           \
    mg_socket_if_connect_udp,                                           \
    mg_socket_if_tcp_send,                                              \
    mg_socket_if_udp_send,                                              \
    mg_socket_if_tcp_recv,                                              \
    mg_socket_if_udp_recv,                                              \
    mg_socket_if_create_conn,                                           \
    mg_socket_if_destroy_conn,                                          \
    mg_socket_if_sock_set,                                              \
    mg_socket_if_get_conn_addr,                                         \
  }
/* clang-format on */

const struct mg_iface_vtable mg_socket_iface_vtable = MG_SOCKET_IFACE_VTABLE;
#if MG_NET_IF == MG_NET_IF_SOCKET
const struct mg_iface_vtable mg_default_iface_vtable = MG_SOCKET_IFACE_VTABLE;
#endif

#endif /* MG_ENABLE_NET_IF_SOCKET */
