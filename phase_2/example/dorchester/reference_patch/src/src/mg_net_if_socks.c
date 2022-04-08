
#if MG_ENABLE_SOCKS

struct socksdata {
  char *proxy_addr;        /* HOST:PORT of the socks5 proxy server */
  struct mg_connection *s; /* Respective connection to the server */
  struct mg_connection *c; /* Connection to the client */
};

static void socks_if_disband(struct socksdata *d) {
  LOG(LL_DEBUG, ("disbanding proxy %p %p", d->c, d->s));
  if (d->c) {
    d->c->flags |= MG_F_SEND_AND_CLOSE;
    d->c->user_data = NULL;
    d->c = NULL;
  }
  if (d->s) {
    d->s->flags |= MG_F_SEND_AND_CLOSE;
    d->s->user_data = NULL;
    d->s = NULL;
  }
}

static void socks_if_relay(struct mg_connection *s) {
  struct socksdata *d = (struct socksdata *) s->user_data;
  if (d == NULL || d->c == NULL || !(s->flags & MG_SOCKS_CONNECT_DONE) ||
      d->s == NULL) {
    return;
  }
  if (s->recv_mbuf.len > 0) mg_if_can_recv_cb(d->c);
  if (d->c->send_mbuf.len > 0 && s->send_mbuf.len == 0) mg_if_can_send_cb(d->c);
}

static void socks_if_handler(struct mg_connection *c, int ev, void *ev_data) {
  struct socksdata *d = (struct socksdata *) c->user_data;
  if (d == NULL) return;
  if (ev == MG_EV_CONNECT) {
    int res = *(int *) ev_data;
    if (res == 0) {
      /* Send handshake to the proxy server */
      unsigned char buf[] = {MG_SOCKS_VERSION, 1, MG_SOCKS_HANDSHAKE_NOAUTH};
      mg_send(d->s, buf, sizeof(buf));
      LOG(LL_DEBUG, ("Sent handshake to %s", d->proxy_addr));
    } else {
      LOG(LL_ERROR, ("Cannot connect to %s: %d", d->proxy_addr, res));
      d->c->flags |= MG_F_CLOSE_IMMEDIATELY;
    }
  } else if (ev == MG_EV_CLOSE) {
    socks_if_disband(d);
  } else if (ev == MG_EV_RECV) {
    /* Handle handshake reply */
    if (!(c->flags & MG_SOCKS_HANDSHAKE_DONE)) {
      /* TODO(lsm): process IPv6 too */
      unsigned char buf[10] = {MG_SOCKS_VERSION, MG_SOCKS_CMD_CONNECT, 0,
                               MG_SOCKS_ADDR_IPV4};
      if (c->recv_mbuf.len < 2) return;
      if ((unsigned char) c->recv_mbuf.buf[1] == MG_SOCKS_HANDSHAKE_FAILURE) {
        LOG(LL_ERROR, ("Server kicked us out"));
        socks_if_disband(d);
        return;
      }
      mbuf_remove(&c->recv_mbuf, 2);
      c->flags |= MG_SOCKS_HANDSHAKE_DONE;

      /* Send connect request */
      memcpy(buf + 4, &d->c->sa.sin.sin_addr, 4);
      memcpy(buf + 8, &d->c->sa.sin.sin_port, 2);
      mg_send(c, buf, sizeof(buf));
      LOG(LL_DEBUG, ("%p Sent connect request", c));
    }
    /* Process connect request */
    if ((c->flags & MG_SOCKS_HANDSHAKE_DONE) &&
        !(c->flags & MG_SOCKS_CONNECT_DONE)) {
      if (c->recv_mbuf.len < 10) return;
      if (c->recv_mbuf.buf[1] != MG_SOCKS_SUCCESS) {
        LOG(LL_ERROR, ("Socks connection error: %d", c->recv_mbuf.buf[1]));
        socks_if_disband(d);
        return;
      }
      mbuf_remove(&c->recv_mbuf, 10);
      c->flags |= MG_SOCKS_CONNECT_DONE;
      LOG(LL_DEBUG, ("%p Connect done %p", c, d->c));
      mg_if_connect_cb(d->c, 0);
    }
    socks_if_relay(c);
  } else if (ev == MG_EV_SEND || ev == MG_EV_POLL) {
    socks_if_relay(c);
  }
}

static void mg_socks_if_connect_tcp(struct mg_connection *c,
                                    const union socket_address *sa) {
  struct socksdata *d = (struct socksdata *) c->iface->data;
  d->c = c;
  d->s = mg_connect(c->mgr, d->proxy_addr, socks_if_handler);
  d->s->user_data = d;
  LOG(LL_DEBUG, ("%p %s %p %p", c, d->proxy_addr, d, d->s));
  (void) sa;
}

static void mg_socks_if_connect_udp(struct mg_connection *c) {
  (void) c;
}

static int mg_socks_if_listen_tcp(struct mg_connection *c,
                                  union socket_address *sa) {
  (void) c;
  (void) sa;
  return 0;
}

static int mg_socks_if_listen_udp(struct mg_connection *c,
                                  union socket_address *sa) {
  (void) c;
  (void) sa;
  return -1;
}

static int mg_socks_if_tcp_send(struct mg_connection *c, const void *buf,
                                size_t len) {
  int res;
  struct socksdata *d = (struct socksdata *) c->iface->data;
  if (d->s == NULL) return -1;
  res = (int) mbuf_append(&d->s->send_mbuf, buf, len);
  DBG(("%p -> %d -> %p", c, res, d->s));
  return res;
}

static int mg_socks_if_udp_send(struct mg_connection *c, const void *buf,
                                size_t len) {
  (void) c;
  (void) buf;
  (void) len;
  return -1;
}

int mg_socks_if_tcp_recv(struct mg_connection *c, void *buf, size_t len) {
  struct socksdata *d = (struct socksdata *) c->iface->data;
  if (d->s == NULL) return -1;
  if (len > d->s->recv_mbuf.len) len = d->s->recv_mbuf.len;
  if (len > 0) {
    memcpy(buf, d->s->recv_mbuf.buf, len);
    mbuf_remove(&d->s->recv_mbuf, len);
  }
  DBG(("%p <- %d <- %p", c, (int) len, d->s));
  return len;
}

int mg_socks_if_udp_recv(struct mg_connection *c, void *buf, size_t len,
                         union socket_address *sa, size_t *sa_len) {
  (void) c;
  (void) buf;
  (void) len;
  (void) sa;
  (void) sa_len;
  return -1;
}

static int mg_socks_if_create_conn(struct mg_connection *c) {
  (void) c;
  return 1;
}

static void mg_socks_if_destroy_conn(struct mg_connection *c) {
  c->iface->vtable->free(c->iface);
  MG_FREE(c->iface);
  c->iface = NULL;
  LOG(LL_DEBUG, ("%p", c));
}

static void mg_socks_if_sock_set(struct mg_connection *c, sock_t sock) {
  (void) c;
  (void) sock;
}

static void mg_socks_if_init(struct mg_iface *iface) {
  (void) iface;
}

static void mg_socks_if_free(struct mg_iface *iface) {
  struct socksdata *d = (struct socksdata *) iface->data;
  LOG(LL_DEBUG, ("%p", iface));
  if (d != NULL) {
    socks_if_disband(d);
    MG_FREE(d->proxy_addr);
    MG_FREE(d);
    iface->data = NULL;
  }
}

static void mg_socks_if_add_conn(struct mg_connection *c) {
  c->sock = INVALID_SOCKET;
}

static void mg_socks_if_remove_conn(struct mg_connection *c) {
  (void) c;
}

static time_t mg_socks_if_poll(struct mg_iface *iface, int timeout_ms) {
  LOG(LL_DEBUG, ("%p", iface));
  (void) iface;
  (void) timeout_ms;
  return (time_t) cs_time();
}

static void mg_socks_if_get_conn_addr(struct mg_connection *c, int remote,
                                      union socket_address *sa) {
  LOG(LL_DEBUG, ("%p", c));
  (void) c;
  (void) remote;
  (void) sa;
}

const struct mg_iface_vtable mg_socks_iface_vtable = {
    mg_socks_if_init,          mg_socks_if_free,
    mg_socks_if_add_conn,      mg_socks_if_remove_conn,
    mg_socks_if_poll,          mg_socks_if_listen_tcp,
    mg_socks_if_listen_udp,    mg_socks_if_connect_tcp,
    mg_socks_if_connect_udp,   mg_socks_if_tcp_send,
    mg_socks_if_udp_send,      mg_socks_if_tcp_recv,
    mg_socks_if_udp_recv,      mg_socks_if_create_conn,
    mg_socks_if_destroy_conn,  mg_socks_if_sock_set,
    mg_socks_if_get_conn_addr,
};

struct mg_iface *mg_socks_mk_iface(struct mg_mgr *mgr, const char *proxy_addr) {
  struct mg_iface *iface = mg_if_create_iface(&mg_socks_iface_vtable, mgr);
  iface->data = MG_CALLOC(1, sizeof(struct socksdata));
  ((struct socksdata *) iface->data)->proxy_addr = strdup(proxy_addr);
  return iface;
}

#endif
