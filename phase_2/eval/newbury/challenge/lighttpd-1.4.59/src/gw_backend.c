/*
 * gw_backend - gateway backend code shared by dynamic socket backends
 *
 * Copyright(c) 2017 Glenn Strauss gstrauss()gluelogic.com  All rights reserved
 * License: BSD 3-clause (same as lighttpd)
 */
#include "first.h"

#include "gw_backend.h"

#include <sys/types.h>
#include <sys/stat.h>
#include "sys-socket.h"
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "base.h"
#include "algo_md.h"
#include "array.h"
#include "buffer.h"
#include "chunk.h"
#include "fdevent.h"
#include "http_header.h"
#include "log.h"
#include "sock_addr.h"




#include "status_counter.h"

__attribute_noinline__
static int * gw_status_get_counter(gw_host *host, gw_proc *proc, const char *tag, size_t tlen) {
    /*(At the cost of some memory, could prepare strings for host and for proc
     * so that here we would copy ready made string for proc (or if NULL,
     * for host), and then append tag to produce key)*/
    char label[288];
    size_t llen = sizeof("gw.backend.")-1, len;
    memcpy(label, "gw.backend.", llen);

    len = buffer_string_length(host->id);
    force_assert(len < sizeof(label) - llen);
    memcpy(label+llen, host->id->ptr, len);
    llen += len;

    if (proc) {
        force_assert(llen < sizeof(label) - (LI_ITOSTRING_LENGTH + 1));
        label[llen++] = '.';
        len = li_utostrn(label+llen, LI_ITOSTRING_LENGTH, proc->id);
        llen += len;
    }

    force_assert(tlen < sizeof(label) - llen);
    memcpy(label+llen, tag, tlen);
    llen += tlen;
    label[llen] = '\0';

    return status_counter_get_counter(label, llen);
}

static void gw_proc_tag_inc(gw_host *host, gw_proc *proc, const char *tag, size_t len) {
    ++(*gw_status_get_counter(host, proc, tag, len));
}

static void gw_proc_load_inc(gw_host *host, gw_proc *proc) {
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".load")) = ++proc->load;

    status_counter_inc(CONST_STR_LEN("gw.active-requests"));
}

static void gw_proc_load_dec(gw_host *host, gw_proc *proc) {
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".load")) = --proc->load;

    status_counter_dec(CONST_STR_LEN("gw.active-requests"));
}

static void gw_host_assign(gw_host *host) {
    *gw_status_get_counter(host, NULL, CONST_STR_LEN(".load")) = ++host->load;
}

static void gw_host_reset(gw_host *host) {
    *gw_status_get_counter(host, NULL, CONST_STR_LEN(".load")) = --host->load;
}

static int gw_status_init(gw_host *host, gw_proc *proc) {
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".disabled")) = 0;
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".died")) = 0;
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".overloaded")) = 0;
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".connected")) = 0;
    *gw_status_get_counter(host, proc, CONST_STR_LEN(".load")) = 0;

    *gw_status_get_counter(host, NULL, CONST_STR_LEN(".load")) = 0;

    return 0;
}




static void gw_proc_set_state(gw_host *host, gw_proc *proc, int state) {
    if ((int)proc->state == state) return;
    if (proc->state == PROC_STATE_RUNNING) {
        --host->active_procs;
    } else if (state == PROC_STATE_RUNNING) {
        ++host->active_procs;
    }
    proc->state = state;
}


static gw_proc *gw_proc_init(void) {
    gw_proc *f = calloc(1, sizeof(*f));
    force_assert(f);

    f->unixsocket = buffer_init();
    f->connection_name = buffer_init();

    f->prev = NULL;
    f->next = NULL;
    f->state = PROC_STATE_DIED;

    return f;
}

static void gw_proc_free(gw_proc *f) {
    if (!f) return;

    gw_proc_free(f->next);

    buffer_free(f->unixsocket);
    buffer_free(f->connection_name);
    free(f->saddr);

    free(f);
}

static gw_host *gw_host_init(void) {
    gw_host *f = calloc(1, sizeof(*f));
    force_assert(f);
    return f;
}

static void gw_host_free(gw_host *h) {
    if (!h) return;
    if (h->refcount) {
        --h->refcount;
        return;
    }

    gw_proc_free(h->first);
    gw_proc_free(h->unused_procs);

    for (uint32_t i = 0; i < h->args.used; ++i) free(h->args.ptr[i]);
    free(h->args.ptr);
    free(h);
}

static gw_exts *gw_extensions_init(void) {
    gw_exts *f = calloc(1, sizeof(*f));
    force_assert(f);
    return f;
}

static void gw_extensions_free(gw_exts *f) {
    if (!f) return;
    for (uint32_t i = 0; i < f->used; ++i) {
        gw_extension *fe = f->exts+i;
        for (uint32_t j = 0; j < fe->used; ++j) {
            gw_host_free(fe->hosts[j]);
        }
        free(fe->hosts);
    }
    free(f->exts);
    free(f);
}

static int gw_extension_insert(gw_exts *ext, const buffer *key, gw_host *fh) {
    gw_extension *fe = NULL;
    for (uint32_t i = 0; i < ext->used; ++i) {
        if (buffer_is_equal(key, &ext->exts[i].key)) {
            fe = ext->exts+i;
            break;
        }
    }

    if (NULL == fe) {
        if (ext->used == ext->size) {
            ext->size += 8;
            ext->exts = realloc(ext->exts, ext->size * sizeof(gw_extension));
            force_assert(ext->exts);
            memset(ext->exts + ext->used, 0, 8 * sizeof(gw_extension));
        }
        fe = ext->exts + ext->used++;
        fe->last_used_ndx = -1;
        buffer *b;
        *(const buffer **)&b = &fe->key;
        memcpy(b, key, sizeof(buffer)); /*(copy; not later free'd)*/
    }

    if (fe->size == fe->used) {
        fe->size += 4;
        fe->hosts = realloc(fe->hosts, fe->size * sizeof(*(fe->hosts)));
        force_assert(fe->hosts);
    }

    fe->hosts[fe->used++] = fh;
    return 0;
}

static void gw_proc_connect_success(gw_host *host, gw_proc *proc, int debug, request_st * const r) {
    gw_proc_tag_inc(host, proc, CONST_STR_LEN(".connected"));
    proc->last_used = log_epoch_secs;

    if (debug) {
        log_error(r->conf.errh, __FILE__, __LINE__,
          "got proc: pid: %d socket: %s load: %d",
          proc->pid, proc->connection_name->ptr, proc->load);
    }
}

__attribute_cold__
static void gw_proc_connect_error(request_st * const r, gw_host *host, gw_proc *proc, pid_t pid, int errnum, int debug) {
    const time_t cur_ts = log_epoch_secs;
    log_error_st * const errh = r->conf.errh;
    log_error(errh, __FILE__, __LINE__,
      "establishing connection failed: socket: %s: %s",
      proc->connection_name->ptr, strerror(errnum));

    if (!proc->is_local) {
        proc->disabled_until = cur_ts + host->disable_time;
        gw_proc_set_state(host, proc, PROC_STATE_OVERLOADED);
    }
    else if (proc->pid == pid && proc->state == PROC_STATE_RUNNING) {
        /* several requests from lighttpd might reference the same proc
         *
         * Only one of them should mark the proc
         * and all other ones should just take a new one.
         *
         * If a new proc was started with the old struct, this might
         * otherwise lead to marking a perfectly good proc as dead
         */
        log_error(errh, __FILE__, __LINE__,
          "backend error; we'll disable for %d"
          "secs and send the request to another backend instead:"
          "load: %d", host->disable_time, host->load);
        if (EAGAIN == errnum) {
            /* - EAGAIN: cool down the backend; it is overloaded */
          #ifdef __linux__
            log_error(errh, __FILE__, __LINE__,
              "If this happened on Linux: You have run out of local ports. "
              "Check the manual, section Performance how to handle this.");
          #endif
            if (debug) {
                log_error(errh, __FILE__, __LINE__,
                  "This means that you have more incoming requests than your "
                  "FastCGI backend can handle in parallel.  It might help to "
                  "spawn more FastCGI backends or PHP children; if not, "
                  "decrease server.max-connections.  The load for this FastCGI "
                  "backend %s is %d", proc->connection_name->ptr, proc->load);
            }
            proc->disabled_until = cur_ts + host->disable_time;
            gw_proc_set_state(host, proc, PROC_STATE_OVERLOADED);
        }
        else {
            /* we got a hard error from the backend like
             * - ECONNREFUSED for tcp-ip sockets
             * - ENOENT for unix-domain-sockets
             */
          #if 0
            gw_proc_set_state(host, proc, PROC_STATE_DIED_WAIT_FOR_PID);
          #else  /* treat as overloaded (future: unless we send kill() signal)*/
            proc->disabled_until = cur_ts + host->disable_time;
            gw_proc_set_state(host, proc, PROC_STATE_OVERLOADED);
          #endif
        }
    }

    if (EAGAIN == errnum) {
        gw_proc_tag_inc(host, proc, CONST_STR_LEN(".overloaded"));
    }
    else {
        gw_proc_tag_inc(host, proc, CONST_STR_LEN(".died"));
    }
}

static void gw_proc_release(gw_host *host, gw_proc *proc, int debug, log_error_st *errh) {
    gw_proc_load_dec(host, proc);

    if (debug) {
        log_error(errh, __FILE__, __LINE__,
          "released proc: pid: %d socket: %s load: %u",
          proc->pid, proc->connection_name->ptr, proc->load);
    }
}

static void gw_proc_check_enable(gw_host * const host, gw_proc * const proc, log_error_st * const errh) {
    if (log_epoch_secs <= proc->disabled_until) return;
    if (proc->state != PROC_STATE_OVERLOADED) return;

    gw_proc_set_state(host, proc, PROC_STATE_RUNNING);

    log_error(errh, __FILE__, __LINE__,
      "gw-server re-enabled: %s %s %hu %s",
      proc->connection_name->ptr,
      host->host ? host->host->ptr : "", host->port,
      host->unixsocket && host->unixsocket->ptr ? host->unixsocket->ptr : "");
}

static void gw_proc_waitpid_log(const gw_host * const host, const gw_proc * const proc, log_error_st * const errh, const int status) {
    if (WIFEXITED(status)) {
        if (proc->state != PROC_STATE_KILLED) {
            log_error(errh, __FILE__, __LINE__,
              "child exited: %d %s",
              WEXITSTATUS(status), proc->connection_name->ptr);
        }
    } else if (WIFSIGNALED(status)) {
        if (WTERMSIG(status) != SIGTERM && WTERMSIG(status) != SIGINT
            && WTERMSIG(status) != host->kill_signal) {
            log_error(errh, __FILE__, __LINE__,
              "child signalled: %d", WTERMSIG(status));
        }
    } else {
        log_error(errh, __FILE__, __LINE__,
          "child died somehow: %d", status);
    }
}

static int gw_proc_waitpid(gw_host *host, gw_proc *proc, log_error_st *errh) {
    int rc, status;

    if (!proc->is_local) return 0;
    if (proc->pid <= 0) return 0;

    rc = fdevent_waitpid(proc->pid, &status, 1);
    if (0 == rc) return 0; /* child still running */

    /* child terminated */
    if (-1 == rc) {
        /* EINVAL or ECHILD no child processes */
        /* should not happen; someone else has cleaned up for us */
        log_perror(errh, __FILE__, __LINE__,
          "pid %d %d not found", proc->pid, proc->state);
    }
    else {
        gw_proc_waitpid_log(host, proc, errh, status);
    }

    proc->pid = 0;
    if (proc->state != PROC_STATE_KILLED)
        proc->disabled_until = log_epoch_secs;
    gw_proc_set_state(host, proc, PROC_STATE_DIED);
    return 1;
}

static int gw_proc_sockaddr_init(gw_host * const host, gw_proc * const proc, log_error_st * const errh) {
    sock_addr addr;
    socklen_t addrlen;

    if (!buffer_string_is_empty(proc->unixsocket)) {
        if (1 != sock_addr_from_str_hints(&addr,&addrlen,proc->unixsocket->ptr,
                                          AF_UNIX, 0, errh)) {
            errno = EINVAL;
            return -1;
        }
        buffer_copy_string_len(proc->connection_name, CONST_STR_LEN("unix:"));
        buffer_append_string_buffer(proc->connection_name, proc->unixsocket);
    }
    else {
        /*(note: name resolution here is *blocking* if IP string not supplied)*/
        if (1 != sock_addr_from_str_hints(&addr, &addrlen, host->host->ptr,
                                          0, proc->port, errh)) {
            errno = EINVAL;
            return -1;
        }
        else if (host->host->size) {
            /*(skip if constant string set in gw_set_defaults_backend())*/
            /* overwrite host->host buffer with IP addr string so that
             * any further use of gw_host does not block on DNS lookup */
            buffer *h;
            *(const buffer **)&h = host->host;
            sock_addr_inet_ntop_copy_buffer(h, &addr);
            host->family = sock_addr_get_family(&addr);
        }
        buffer_copy_string_len(proc->connection_name, CONST_STR_LEN("tcp:"));
        buffer_append_string_buffer(proc->connection_name, host->host);
        buffer_append_string_len(proc->connection_name, CONST_STR_LEN(":"));
        buffer_append_int(proc->connection_name, proc->port);
    }

    if (NULL != proc->saddr && proc->saddrlen < addrlen) {
        free(proc->saddr);
        proc->saddr = NULL;
    }
    if (NULL == proc->saddr) {
        proc->saddr = (struct sockaddr *)malloc(addrlen);
        force_assert(proc->saddr);
    }
    proc->saddrlen = addrlen;
    memcpy(proc->saddr, &addr, addrlen);
    return 0;
}

static int env_add(char_array *env, const char *key, size_t key_len, const char *val, size_t val_len) {
    char *dst;

    if (!key || !val) return -1;

    dst = malloc(key_len + val_len + 3);
    force_assert(dst);
    memcpy(dst, key, key_len);
    dst[key_len] = '=';
    memcpy(dst + key_len + 1, val, val_len + 1); /* add the \0 from the value */

    for (uint32_t i = 0; i < env->used; ++i) {
      #ifdef __COVERITY__
        force_assert(env->ptr); /*(non-NULL if env->used != 0)*/
      #endif
        if (0 == strncmp(dst, env->ptr[i], key_len + 1)) {
            free(env->ptr[i]);
            env->ptr[i] = dst;
            return 0;
        }
    }

    if (env->size <= env->used + 1) {
        env->size += 16;
        env->ptr = realloc(env->ptr, env->size * sizeof(*env->ptr));
        force_assert(env->ptr);
    }

  #ifdef __COVERITY__
    force_assert(env->ptr); /*(non-NULL if env->used != 0; guaranteed above)*/
  #endif
    env->ptr[env->used++] = dst;

    return 0;
}

static int gw_spawn_connection(gw_host * const host, gw_proc * const proc, log_error_st * const errh, int debug) {
    int gw_fd;
    int status;
    struct timeval tv = { 0, 10 * 1000 };

    if (debug) {
        log_error(errh, __FILE__, __LINE__,
          "new proc, socket: %hu %s",
          proc->port, proc->unixsocket->ptr ? proc->unixsocket->ptr : "");
    }

    gw_fd = fdevent_socket_cloexec(proc->saddr->sa_family, SOCK_STREAM, 0);
    if (-1 == gw_fd) {
        log_perror(errh, __FILE__, __LINE__, "socket()");
        return -1;
    }

    do {
        status = connect(gw_fd, proc->saddr, proc->saddrlen);
    } while (-1 == status && errno == EINTR);

    if (-1 == status && errno != ENOENT
        && !buffer_string_is_empty(proc->unixsocket)) {
        log_perror(errh, __FILE__, __LINE__,
          "unlink %s after connect failed", proc->unixsocket->ptr);
        unlink(proc->unixsocket->ptr);
    }

    close(gw_fd);

    if (-1 == status) {
        /* server is not up, spawn it  */
        char_array env;
        uint32_t i;
        int dfd = -1;

        /* reopen socket */
        gw_fd = fdevent_socket_cloexec(proc->saddr->sa_family, SOCK_STREAM, 0);
        if (-1 == gw_fd) {
            log_perror(errh, __FILE__, __LINE__, "socket()");
            return -1;
        }

        if (fdevent_set_so_reuseaddr(gw_fd, 1) < 0) {
            log_perror(errh, __FILE__, __LINE__, "socketsockopt()");
            close(gw_fd);
            return -1;
        }

        /* create socket */
        if (-1 == bind(gw_fd, proc->saddr, proc->saddrlen)) {
            log_perror(errh, __FILE__, __LINE__,
              "bind failed for: %s", proc->connection_name->ptr);
            close(gw_fd);
            return -1;
        }

        if (-1 == listen(gw_fd, host->listen_backlog)) {
            log_perror(errh, __FILE__, __LINE__, "listen()");
            close(gw_fd);
            return -1;
        }

        {
            /* create environment */
            env.ptr = NULL;
            env.size = 0;
            env.used = 0;

            /* build clean environment */
            if (host->bin_env_copy && host->bin_env_copy->used) {
                for (i = 0; i < host->bin_env_copy->used; ++i) {
                    data_string *ds=(data_string *)host->bin_env_copy->data[i];
                    char *ge;

                    if (NULL != (ge = getenv(ds->value.ptr))) {
                        env_add(&env,CONST_BUF_LEN(&ds->value),ge,strlen(ge));
                    }
                }
            } else {
                char ** const e = fdevent_environ();
                for (i = 0; e[i]; ++i) {
                    char *eq;

                    if (NULL != (eq = strchr(e[i], '='))) {
                        env_add(&env, e[i], eq - e[i], eq+1, strlen(eq+1));
                    }
                }
            }

            /* create environment */
            if (host->bin_env) {
                for (i = 0; i < host->bin_env->used; ++i) {
                    data_string *ds = (data_string *)host->bin_env->data[i];
                    env_add(&env, CONST_BUF_LEN(&ds->key),
                                  CONST_BUF_LEN(&ds->value));
                }
            }

            for (i = 0; i < env.used; ++i) {
                /* search for PHP_FCGI_CHILDREN */
                if (0 == strncmp(env.ptr[i], "PHP_FCGI_CHILDREN=",
                                      sizeof("PHP_FCGI_CHILDREN=")-1)) {
                    break;
                }
            }

            /* not found, add a default */
            if (i == env.used) {
                env_add(&env, CONST_STR_LEN("PHP_FCGI_CHILDREN"),
                              CONST_STR_LEN("1"));
            }

            env.ptr[env.used] = NULL;
        }

        dfd = fdevent_open_dirname(host->args.ptr[0], 1); /* permit symlinks */
        if (-1 == dfd) {
            log_perror(errh, __FILE__, __LINE__,
              "open dirname failed: %s", host->args.ptr[0]);
        }

        /*(FCGI_LISTENSOCK_FILENO == STDIN_FILENO == 0)*/
        proc->pid = (dfd >= 0)
          ? fdevent_fork_execve(host->args.ptr[0], host->args.ptr,
                                env.ptr, gw_fd, -1, -1, dfd)
          : -1;

        for (i = 0; i < env.used; ++i) free(env.ptr[i]);
        free(env.ptr);
        if (-1 != dfd) close(dfd);
        close(gw_fd);

        if (-1 == proc->pid) {
            log_error(errh, __FILE__, __LINE__,
              "gw-backend failed to start: %s", host->bin_path->ptr);
            proc->pid = 0;
            proc->disabled_until = log_epoch_secs;
            return -1;
        }

        /* register process */
        proc->last_used = log_epoch_secs;
        proc->is_local = 1;

        /* wait */
        select(0, NULL, NULL, NULL, &tv);

        if (0 != gw_proc_waitpid(host, proc, errh)) {
            log_error(errh, __FILE__, __LINE__,
              "gw-backend failed to start: %s", host->bin_path->ptr);
            log_error(errh, __FILE__, __LINE__,
              "If you're trying to run your app as a FastCGI backend, make "
              "sure you're using the FastCGI-enabled version.  If this is PHP "
              "on Gentoo, add 'fastcgi' to the USE flags.  If this is PHP, try "
              "removing the bytecode caches for now and try again.");
            return -1;
        }
    } else {
        proc->is_local = 0;
        proc->pid = 0;

        if (debug) {
            log_error(errh, __FILE__, __LINE__,
              "(debug) socket is already used; won't spawn: %s",
              proc->connection_name->ptr);
        }
    }

    gw_proc_set_state(host, proc, PROC_STATE_RUNNING);
    return 0;
}

static void gw_proc_spawn(gw_host * const host, log_error_st * const errh, const int debug) {
    gw_proc *proc;
    for (proc = host->unused_procs; proc; proc = proc->next) {
        /* (proc->pid <= 0 indicates PROC_STATE_DIED, not PROC_STATE_KILLED) */
        if (proc->pid > 0) continue;
        /* (do not attempt to spawn another proc if a proc just exited) */
        if (proc->disabled_until >= log_epoch_secs) return;
        break;
    }
    if (proc) {
        if (proc == host->unused_procs)
            host->unused_procs = proc->next;
        else
            proc->prev->next = proc->next;

        if (proc->next) {
            proc->next->prev = proc->prev;
            proc->next = NULL;
        }

        proc->prev = NULL;
    } else {
        proc = gw_proc_init();
        proc->id = host->max_id++;
    }

    ++host->num_procs;

    if (buffer_string_is_empty(host->unixsocket)) {
        proc->port = host->port + proc->id;
    } else {
        buffer_copy_buffer(proc->unixsocket, host->unixsocket);
        buffer_append_string_len(proc->unixsocket, CONST_STR_LEN("-"));
        buffer_append_int(proc->unixsocket, proc->id);
    }

    if (0 != gw_proc_sockaddr_init(host, proc, errh)) {
        /*(should not happen if host->host validated at startup,
         * and translated from name to IP address at startup)*/
        log_error(errh, __FILE__, __LINE__,
          "ERROR: spawning backend failed.");
        --host->num_procs;
        if (proc->id == host->max_id-1) --host->max_id;
        gw_proc_free(proc);
    } else if (gw_spawn_connection(host, proc, errh, debug)) {
        log_error(errh, __FILE__, __LINE__,
          "ERROR: spawning backend failed.");
        proc->next = host->unused_procs;
        if (host->unused_procs)
            host->unused_procs->prev = proc;
        host->unused_procs = proc;
    } else {
        proc->next = host->first;
        if (host->first)
            host->first->prev = proc;
        host->first = proc;
    }
}

static void gw_proc_kill(gw_host *host, gw_proc *proc) {
    if (proc->next) proc->next->prev = proc->prev;
    if (proc->prev) proc->prev->next = proc->next;

    if (proc->prev == NULL) host->first = proc->next;

    proc->prev = NULL;
    proc->next = host->unused_procs;
    proc->disabled_until = 0;

    if (host->unused_procs)
        host->unused_procs->prev = proc;
    host->unused_procs = proc;

    kill(proc->pid, host->kill_signal);

    gw_proc_set_state(host, proc, PROC_STATE_KILLED);

    --host->num_procs;
}

static gw_host * unixsocket_is_dup(gw_plugin_data *p, const buffer *unixsocket) {
    if (NULL == p->cvlist) return NULL;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        gw_plugin_config *conf = NULL;
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* xxxxx.server */
                if (cpv->vtype == T_CONFIG_LOCAL) conf = cpv->v.v;
                break;
              default:
                break;
            }
        }

        if (NULL == conf || NULL == conf->exts) continue;

        gw_exts *exts = conf->exts;
        for (uint32_t j = 0; j < exts->used; ++j) {
            gw_extension *ex = exts->exts+j;
            for (uint32_t n = 0; n < ex->used; ++n) {
                gw_host *host = ex->hosts[n];
                if (!buffer_string_is_empty(host->unixsocket)
                    && buffer_is_equal(host->unixsocket, unixsocket)
                    && !buffer_string_is_empty(host->bin_path))
                    return host;
            }
        }
    }

    return NULL;
}

static int parse_binpath(char_array *env, const buffer *b) {
    char *start = b->ptr;
    char c;
    /* search for spaces */
    for (size_t i = 0; i < buffer_string_length(b); ++i) {
        switch(b->ptr[i]) {
        case ' ':
        case '\t':
            /* a WS, stop here and copy the argument */

            if (env->size == env->used) {
                env->size += 16;
                env->ptr = realloc(env->ptr, env->size * sizeof(*env->ptr));
            }

            c = b->ptr[i];
            b->ptr[i] = '\0';
            env->ptr[env->used++] = strdup(start);
            b->ptr[i] = c;

            start = b->ptr + i + 1;
            break;
        default:
            break;
        }
    }

    if (env->size == env->used) { /*need one extra for terminating NULL*/
        env->size += 16;
        env->ptr = realloc(env->ptr, env->size * sizeof(*env->ptr));
    }

    /* the rest */
    env->ptr[env->used++] = strdup(start);

    if (env->size == env->used) { /*need one extra for terminating NULL*/
        env->size += 16;
        env->ptr = realloc(env->ptr, env->size * sizeof(*env->ptr));
    }

    /* terminate */
    env->ptr[env->used++] = NULL;

    return 0;
}

enum {
  GW_BALANCE_LEAST_CONNECTION,
  GW_BALANCE_RR,
  GW_BALANCE_HASH,
  GW_BALANCE_STICKY
};

__attribute_noinline__
static uint32_t
gw_hash(const char *str, const uint32_t len, uint32_t hash)
{
    return djbhash(str, len, hash);
}

static gw_host * gw_host_get(request_st * const r, gw_extension *extension, int balance, int debug) {
    gw_host *host;
    buffer *dst_addr_buf;
    uint32_t last_max = UINT32_MAX;
    uint32_t base_hash = DJBHASH_INIT;
    int max_usage = INT_MAX;
    int ndx = -1;
    uint32_t k;

    if (extension->used <= 1) {
        if (1 == extension->used && extension->hosts[0]->active_procs > 0) {
            ndx = 0;
        }
    } else switch(balance) {
    case GW_BALANCE_HASH:
        /* hash balancing */

        if (debug) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "proxy - used hash balancing, hosts: %u", extension->used);
        }

        base_hash = gw_hash(CONST_BUF_LEN(&r->uri.path), base_hash);
        base_hash = gw_hash(CONST_BUF_LEN(&r->uri.authority), base_hash);

        for (k = 0, ndx = -1, last_max = UINT32_MAX; k < extension->used; ++k) {
            uint32_t cur_max;
            host = extension->hosts[k];
            if (0 == host->active_procs) continue;

            /* future: hash of host->host could be cached separately, mixed in*/
            cur_max = gw_hash(CONST_BUF_LEN(host->host), base_hash);

            if (debug) {
                log_error(r->conf.errh, __FILE__, __LINE__,
                  "proxy - election: %s %s %s %u", r->uri.path.ptr,
                  host->host ? host->host->ptr : "",
                  r->uri.authority.ptr, cur_max);
            }

            if (last_max < cur_max || last_max == UINT32_MAX) {
                last_max = cur_max;
                ndx = k;
            }
        }

        break;
    case GW_BALANCE_LEAST_CONNECTION:
        /* fair balancing */
        if (debug) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "proxy - used least connection");
        }

        for (k = 0, ndx = -1, max_usage = INT_MAX; k < extension->used; ++k) {
            host = extension->hosts[k];
            if (0 == host->active_procs) continue;

            if (host->load < max_usage) {
                max_usage = host->load;
                ndx = k;
            }
        }

        break;
    case GW_BALANCE_RR:
        /* round robin */
        if (debug) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "proxy - used round-robin balancing");
        }

        /* just to be sure */
        force_assert(extension->used < INT_MAX);

        host = extension->hosts[0];

        /* Use last_used_ndx from first host in list */
        k = extension->last_used_ndx;
        ndx = k + 1; /* use next host after the last one */
        if (ndx < 0) ndx = 0;

        /* Search first active host after last_used_ndx */
        while (ndx < (int) extension->used
               && 0 == (host = extension->hosts[ndx])->active_procs) ++ndx;

        if (ndx >= (int) extension->used) {
            /* didn't find a higher id, wrap to the start */
            for (ndx = 0; ndx <= (int) k; ++ndx) {
                host = extension->hosts[ndx];
                if (0 != host->active_procs) break;
            }

            /* No active host found */
            if (0 == host->active_procs) ndx = -1;
        }

        /* Save new index for next round */
        extension->last_used_ndx = ndx;

        break;
    case GW_BALANCE_STICKY:
        /* source sticky balancing */
        dst_addr_buf = r->con->dst_addr_buf;

        if (debug) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "proxy - used sticky balancing, hosts: %u", extension->used);
        }

        base_hash = gw_hash(CONST_BUF_LEN(dst_addr_buf), base_hash);

        for (k = 0, ndx = -1, last_max = UINT32_MAX; k < extension->used; ++k) {
            unsigned long cur_max;
            host = extension->hosts[k];

            if (0 == host->active_procs) continue;

            /* future: hash of host->host could be cached separately, mixed in*/
            cur_max = gw_hash(CONST_BUF_LEN(host->host), base_hash)
                    + host->port;

            if (debug) {
                log_error(r->conf.errh, __FILE__, __LINE__,
                  "proxy - election: %s %s %hu %ld", dst_addr_buf->ptr,
                  host->host ? host->host->ptr : "",
                  host->port, cur_max);
            }

            if (last_max < cur_max || last_max == UINT32_MAX) {
                last_max = cur_max;
                ndx = k;
            }
        }

        break;
    default:
        break;
    }

    if (-1 != ndx) {
        /* found a server */
        host = extension->hosts[ndx];

        if (debug) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "gw - found a host %s %hu",
              host->host ? host->host->ptr : "", host->port);
        }

        return host;
    } else if (0 == r->con->srv->srvconf.max_worker) {
        /* special-case adaptive spawning and 0 == host->min_procs */
        for (k = 0; k < extension->used; ++k) {
            host = extension->hosts[k];
            if (0 == host->min_procs && 0 == host->num_procs
                && !buffer_string_is_empty(host->bin_path)) {
                gw_proc_spawn(host, r->con->srv->errh, debug);
                if (host->num_procs) return host;
            }
        }
    }

    /* all hosts are down */
    /* sorry, we don't have a server alive for this ext */
    r->http_status = 503; /* Service Unavailable */
    r->handler_module = NULL;

    /* only send the 'no handler' once */
    if (!extension->note_is_sent) {
        extension->note_is_sent = 1;
        log_error(r->conf.errh, __FILE__, __LINE__,
          "all handlers for %s?%.*s on %s are down.",
          r->uri.path.ptr, BUFFER_INTLEN_PTR(&r->uri.query),
          extension->key.ptr);
    }

    return NULL;
}

static int gw_establish_connection(request_st * const r, gw_host *host, gw_proc *proc, pid_t pid, int gw_fd, int debug) {
    if (-1 == connect(gw_fd, proc->saddr, proc->saddrlen)) {
        if (errno == EINPROGRESS || errno == EALREADY || errno == EINTR) {
            if (debug > 2) {
                log_error(r->conf.errh, __FILE__, __LINE__,
                  "connect delayed; will continue later: %s",
                  proc->connection_name->ptr);
            }

            return 1;
        } else {
            gw_proc_connect_error(r, host, proc, pid, errno, debug);
            return -1;
        }
    }

    if (debug > 1) {
        log_error(r->conf.errh, __FILE__, __LINE__,
          "connect succeeded: %d", gw_fd);
    }

    return 0;
}

static void gw_restart_dead_procs(gw_host * const host, log_error_st * const errh, const int debug, const int trigger) {
    for (gw_proc *proc = host->first; proc; proc = proc->next) {
        if (debug > 2) {
            log_error(errh, __FILE__, __LINE__,
              "proc: %s %d %d %d %d", proc->connection_name->ptr,
              proc->state, proc->is_local, proc->load, proc->pid);
        }

        switch (proc->state) {
        case PROC_STATE_RUNNING:
            break;
        case PROC_STATE_OVERLOADED:
            gw_proc_check_enable(host, proc, errh);
            break;
        case PROC_STATE_KILLED:
            if (trigger && ++proc->disabled_until > 4) {
                int sig = (proc->disabled_until <= 8)
                  ? host->kill_signal
                  : proc->disabled_until <= 16 ? SIGTERM : SIGKILL;
                kill(proc->pid, sig);
            }
            break;
        case PROC_STATE_DIED_WAIT_FOR_PID:
            /*(state should not happen in workers if server.max-worker > 0)*/
            /*(if PROC_STATE_DIED_WAIT_FOR_PID is used in future, might want
             * to save proc->disabled_until before gw_proc_waitpid() since
             * gw_proc_waitpid will set proc->disabled_until to log_epoch_secs,
             * and so process will not be restarted below until one sec later)*/
            if (0 == gw_proc_waitpid(host, proc, errh)) {
                gw_proc_check_enable(host, proc, errh);
            }

            if (proc->state != PROC_STATE_DIED) break;
            __attribute_fallthrough__/*(we have a dead proc now)*/

        case PROC_STATE_DIED:
            /* local procs get restarted by us,
             * remote ones hopefully by the admin */

            if (!buffer_string_is_empty(host->bin_path)) {
                /* we still have connections bound to this proc,
                 * let them terminate first */
                if (proc->load != 0) break;

                /* avoid spinning if child exits too quickly */
                if (proc->disabled_until >= log_epoch_secs) break;

                /* restart the child */

                if (debug) {
                    log_error(errh, __FILE__, __LINE__,
                      "--- gw spawning"
                      "\n\tsocket %s"
                      "\n\tcurrent: 1 / %u",
                      proc->connection_name->ptr, host->max_procs);
                }

                if (gw_spawn_connection(host, proc, errh, debug)) {
                    log_error(errh, __FILE__, __LINE__,
                      "ERROR: spawning gw failed.");
                }
            } else {
                gw_proc_check_enable(host, proc, errh);
            }
            break;
        }
    }
}




#include "base.h"
#include "connections.h"
#include "response.h"


/* ok, we need a prototype */
static handler_t gw_handle_fdevent(void *ctx, int revents);


static gw_handler_ctx * handler_ctx_init(size_t sz) {
    gw_handler_ctx *hctx = calloc(1, 0 == sz ? sizeof(*hctx) : sz);
    force_assert(hctx);

    /*hctx->response = chunk_buffer_acquire();*//*(allocated when needed)*/

    hctx->request_id = 0;
    hctx->gw_mode = GW_RESPONDER;
    hctx->state = GW_STATE_INIT;
    hctx->proc = NULL;

    hctx->fd = -1;

    hctx->reconnects = 0;
    hctx->send_content_body = 1;

    /*hctx->rb = chunkqueue_init();*//*(allocated when needed)*/
    chunkqueue_init(&hctx->wb);
    hctx->wb_reqlen = 0;

    return hctx;
}

static void handler_ctx_free(gw_handler_ctx *hctx) {
    /* caller MUST have called gw_backend_close(hctx, r) if necessary */
    if (hctx->handler_ctx_free) hctx->handler_ctx_free(hctx);
    chunk_buffer_release(hctx->response);

    if (hctx->rb) chunkqueue_free(hctx->rb);
    chunkqueue_reset(&hctx->wb);

    free(hctx);
}

static void handler_ctx_clear(gw_handler_ctx *hctx) {
    /* caller MUST have called gw_backend_close(hctx, r) if necessary */

    hctx->proc = NULL;
    hctx->host = NULL;
    hctx->ext  = NULL;
    /*hctx->ext_auth is intentionally preserved to flag prior authorizer*/

    hctx->gw_mode = GW_RESPONDER;
    hctx->state = GW_STATE_INIT;
    /*hctx->state_timestamp = 0;*//*(unused; left as-is)*/

    if (hctx->rb) chunkqueue_reset(hctx->rb);
    chunkqueue_reset(&hctx->wb);
    hctx->wb_reqlen = 0;

    if (hctx->response) buffer_clear(hctx->response);

    hctx->fd = -1;
    hctx->reconnects = 0;
    hctx->request_id = 0;
    hctx->send_content_body = 1;

    /*plugin_config conf;*//*(no need to reset for same request)*/

    /*hctx->r           = NULL;*//*(no need to reset for same request)*/
    /*hctx->plugin_data = NULL;*//*(no need to reset for same request)*/
}


void * gw_init(void) {
    return calloc(1, sizeof(gw_plugin_data));
}


void gw_plugin_config_free(gw_plugin_config *s) {
    gw_exts *exts = s->exts;
    if (exts) {
        for (uint32_t j = 0; j < exts->used; ++j) {
            gw_extension *ex = exts->exts+j;
            for (uint32_t n = 0; n < ex->used; ++n) {
                gw_proc *proc;
                gw_host *host = ex->hosts[n];

                for (proc = host->first; proc; proc = proc->next) {
                    if (proc->pid > 0) {
                        kill(proc->pid, host->kill_signal);
                    }

                    if (proc->is_local &&
                        !buffer_string_is_empty(proc->unixsocket)) {
                        unlink(proc->unixsocket->ptr);
                    }
                }

                for (proc = host->unused_procs; proc; proc = proc->next) {
                    if (proc->pid > 0) {
                        kill(proc->pid, host->kill_signal);
                    }
                    if (proc->is_local &&
                        !buffer_string_is_empty(proc->unixsocket)) {
                        unlink(proc->unixsocket->ptr);
                    }
                }
            }
        }

        gw_extensions_free(s->exts);
        gw_extensions_free(s->exts_auth);
        gw_extensions_free(s->exts_resp);
    }
    free(s);
}

void gw_free(void *p_d) {
    gw_plugin_data * const p = p_d;
    if (NULL == p->cvlist) return;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* xxxxx.server */
                if (cpv->vtype == T_CONFIG_LOCAL)
                    gw_plugin_config_free(cpv->v.v);
                break;
              default:
                break;
            }
        }
    }
}

void gw_exts_clear_check_local(gw_exts *exts) {
    for (uint32_t j = 0; j < exts->used; ++j) {
        gw_extension *ex = exts->exts+j;
        for (uint32_t n = 0; n < ex->used; ++n) {
            ex->hosts[n]->check_local = 0;
        }
    }
}

int gw_set_defaults_backend(server *srv, gw_plugin_data *p, const array *a, gw_plugin_config *s, int sh_exec, const char *cpkkey) {
    /* per-module plugin_config MUST have common "base class" gw_plugin_config*/
    /* per-module plugin_data MUST have pointer-compatible common "base class"
     * with gw_plugin_data (stemming from gw_plugin_config compatibility) */

    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("host"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("port"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("socket"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("listen-backlog"),
        T_CONFIG_INT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("bin-path"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("kill-signal"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("check-local"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("mode"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("docroot"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("min-procs"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("max-procs"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("max-load-per-proc"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("idle-timeout"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("disable-time"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("bin-environment"),
        T_CONFIG_ARRAY_KVSTRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("bin-copy-environment"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("broken-scriptfilename"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("strip-request-uri"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("fix-root-scriptname"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("allow-x-send-file"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("x-sendfile"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("x-sendfile-docroot"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("tcp-fin-propagate"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    gw_host *host = NULL;

    p->srv_pid = srv->pid;

    s->exts      = gw_extensions_init();
    s->exts_auth = gw_extensions_init();
    s->exts_resp = gw_extensions_init();
    /*s->balance = GW_BALANCE_LEAST_CONNECTION;*//*(default)*/

    /*
     * gw.server = ( "<ext>" => ( ... ),
     *               "<ext>" => ( ... ) )
     */

    for (uint32_t j = 0; j < a->used; ++j) {
        data_array *da_ext = (data_array *)a->data[j];

        /*
         * da_ext->key == name of the extension
         */

        /*
         * gw.server = ( "<ext>" =>
         *                     ( "<host>" => ( ... ),
         *                       "<host>" => ( ... )
         *                     ),
         *               "<ext>" => ... )
         */

        for (uint32_t n = 0; n < da_ext->value.used; ++n) {
            data_array * const da_host = (data_array *)da_ext->value.data[n];

            if (da_host->type != TYPE_ARRAY
                || !array_is_kvany(&da_host->value)){
                log_error(srv->errh, __FILE__, __LINE__,
                  "unexpected value for gw.server near [%s](string); "
                  "expected ( \"ext\" => "
                  "( \"backend-label\" => ( \"key\" => \"value\" )))",
                  da_host->key.ptr ? da_host->key.ptr : "");
                goto error;
            }

            config_plugin_value_t cvlist[sizeof(cpk)/sizeof(cpk[0])+1];
            memset(cvlist, 0, sizeof(cvlist));

            array *ca = &da_host->value;
            if (!config_plugin_values_init_block(srv, ca, cpk, cpkkey, cvlist))
                goto error;

            unsigned short host_mode = GW_RESPONDER;

            host = gw_host_init();
            host->id = &da_host->key;
            host->check_local  = 1;
            host->min_procs    = 4;
            host->max_procs    = 4;
            host->max_load_per_proc = 1;
            host->idle_timeout = 60;
            host->disable_time = 1;
            host->break_scriptfilename_for_php = 0;
            host->kill_signal = SIGTERM;
            host->fix_root_path_name = 0;
            host->listen_backlog = 1024;
            host->xsendfile_allow = 0;
            host->refcount = 0;

            config_plugin_value_t *cpv = cvlist;
            for (; -1 != cpv->k_id; ++cpv) {
                switch (cpv->k_id) {
                  case 0: /* host */
                    host->host = cpv->v.b;
                    break;
                  case 1: /* port */
                    host->port = cpv->v.shrt;
                    break;
                  case 2: /* socket */
                    host->unixsocket = cpv->v.b;
                    break;
                  case 3: /* listen-backlog */
                    host->listen_backlog = cpv->v.u;
                    break;
                  case 4: /* bin-path */
                    host->bin_path = cpv->v.b;
                    break;
                  case 5: /* kill-signal */
                    host->kill_signal = cpv->v.shrt;
                    break;
                  case 6: /* check-local */
                    host->check_local = (0 != cpv->v.u);
                    break;
                  case 7: /* mode */
                    if (!buffer_string_is_empty(cpv->v.b)) {
                        const buffer *b = cpv->v.b;
                        if (buffer_eq_slen(b, CONST_STR_LEN("responder")))
                            host_mode = GW_RESPONDER;
                        else if (buffer_eq_slen(b, CONST_STR_LEN("authorizer")))
                            host_mode = GW_AUTHORIZER;
                        else
                            log_error(srv->errh, __FILE__, __LINE__,
                              "WARNING: unknown gw mode: %s "
                              "(ignored, mode set to responder)", b->ptr);
                    }
                    break;
                  case 8: /* docroot */
                    host->docroot = cpv->v.b;
                    break;
                  case 9: /* min-procs */
                    host->min_procs = cpv->v.shrt;
                    break;
                  case 10:/* max-procs */
                    host->max_procs = cpv->v.shrt;
                    break;
                  case 11:/* max-load-per-proc */
                    host->max_load_per_proc = cpv->v.shrt;
                    break;
                  case 12:/* idle-timeout */
                    host->idle_timeout = cpv->v.shrt;
                    break;
                  case 13:/* disable-time */
                    host->disable_time = cpv->v.shrt;
                    break;
                  case 14:/* bin-environment */
                    host->bin_env = cpv->v.a;
                    break;
                  case 15:/* bin-copy-environment */
                    host->bin_env_copy = cpv->v.a;
                    break;
                  case 16:/* broken-scriptfilename */
                    host->break_scriptfilename_for_php = (0 != cpv->v.u);
                    break;
                  case 17:/* strip-request-uri */
                    host->strip_request_uri = cpv->v.b;
                    break;
                  case 18:/* fix-root-scriptname */
                    host->fix_root_path_name = (0 != cpv->v.u);
                    break;
                  case 19:/* allow-x-send-file */
                    host->xsendfile_allow = (0 != cpv->v.u);
                    break;
                  case 20:/* x-sendfile */
                    host->xsendfile_allow = (0 != cpv->v.u);
                    break;
                  case 21:/* x-sendfile-docroot */
                    host->xsendfile_docroot = cpv->v.a;
                    if (cpv->v.a->used) {
                        for (uint32_t k = 0; k < cpv->v.a->used; ++k) {
                            data_string *ds = (data_string *)cpv->v.a->data[k];
                            if (ds->type != TYPE_STRING) {
                                log_error(srv->errh, __FILE__, __LINE__,
                                  "unexpected type for x-sendfile-docroot; "
                                  "expected: \"x-sendfile-docroot\" => "
                                  "( \"/allowed/path\", ... )");
                                goto error;
                            }
                            if (ds->value.ptr[0] != '/') {
                                log_error(srv->errh, __FILE__, __LINE__,
                                  "x-sendfile-docroot paths must begin with "
                                  "'/'; invalid: \"%s\"", ds->value.ptr);
                                goto error;
                            }
                            buffer_path_simplify(&ds->value, &ds->value);
                            buffer_append_slash(&ds->value);
                        }
                    }
                    break;
                  case 22:/* tcp-fin-propagate */
                    host->tcp_fin_propagate = (0 != cpv->v.u);
                    break;
                  default:
                    break;
                }
            }

            for (uint32_t m = 0; m < da_host->value.used; ++m) {
                if (NULL != strchr(da_host->value.data[m]->key.ptr, '_')) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "incorrect directive contains underscore ('_') instead of dash ('-'): %s",
                      da_host->value.data[m]->key.ptr);
                }
            }

            if ((!buffer_string_is_empty(host->host) || host->port)
                && !buffer_string_is_empty(host->unixsocket)) {
                log_error(srv->errh, __FILE__, __LINE__,
                  "either host/port or socket have to be set in: "
                  "%s = (%s => (%s ( ...", cpkkey, da_ext->key.ptr,
                  da_host->key.ptr);

                goto error;
            }

            if (!buffer_string_is_empty(host->host) && *host->host->ptr == '/'
                && buffer_string_is_empty(host->unixsocket)) {
                host->unixsocket = host->host;
            }

            if (!buffer_string_is_empty(host->unixsocket)) {
                /* unix domain socket */
                struct sockaddr_un un;

                if (buffer_string_length(host->unixsocket) + 1 > sizeof(un.sun_path) - 2) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "unixsocket is too long in: %s = (%s => (%s ( ...",
                      cpkkey, da_ext->key.ptr, da_host->key.ptr);

                    goto error;
                }

                if (!buffer_string_is_empty(host->bin_path)) {
                    gw_host *duplicate = unixsocket_is_dup(p, host->unixsocket);
                    if (NULL != duplicate) {
                        if (!buffer_is_equal(host->bin_path, duplicate->bin_path)) {
                            log_error(srv->errh, __FILE__, __LINE__,
                              "duplicate unixsocket path: %s",
                              host->unixsocket->ptr);
                            goto error;
                        }
                        gw_host_free(host);
                        host = duplicate;
                        ++host->refcount;
                    }
                }

                host->family = AF_UNIX;
            } else {
                /* tcp/ip */

                if (buffer_string_is_empty(host->host) &&
                    buffer_string_is_empty(host->bin_path)) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "host or bin-path have to be set in: "
                      "%s = (%s => (%s ( ...", cpkkey, da_ext->key.ptr,
                      da_host->key.ptr);

                    goto error;
                } else if (0 == host->port) {
                    host->port = 80;
                }

                if (buffer_string_is_empty(host->host)) {
                    static const buffer lhost ={CONST_STR_LEN("127.0.0.1")+1,0};
                    host->host = &lhost;
                }

                host->family = (NULL != strchr(host->host->ptr, ':'))
                  ? AF_INET6
                  : AF_INET;
            }

            if (host->refcount) {
                /* already init'd; skip spawning */
            } else if (!buffer_string_is_empty(host->bin_path)) {
                /* a local socket + self spawning */
                struct stat st;
                parse_binpath(&host->args, host->bin_path);
                if (0 != stat(host->args.ptr[0], &st) || !S_ISREG(st.st_mode)
                    || !(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "invalid \"bin-path\" => \"%s\" "
                      "(check that file exists, is regular file, "
                      "and is executable by lighttpd)", host->bin_path->ptr);
                }

                if (sh_exec) {
                    /*(preserve prior behavior for SCGI exec of command)*/
                    /*(admin should really prefer to put
                     * any complex command into a script)*/
                    for (uint32_t m = 0; m < host->args.used; ++m)
                        free(host->args.ptr[m]);
                    free(host->args.ptr);

                    host->args.ptr = calloc(4, sizeof(char *));
                    force_assert(host->args.ptr);
                    host->args.used = 3;
                    host->args.size = 4;
                    host->args.ptr[0] = malloc(sizeof("/bin/sh"));
                    force_assert(host->args.ptr[0]);
                    memcpy(host->args.ptr[0], "/bin/sh", sizeof("/bin/sh"));
                    host->args.ptr[1] = malloc(sizeof("-c"));
                    force_assert(host->args.ptr[1]);
                    memcpy(host->args.ptr[1], "-c", sizeof("-c"));
                    host->args.ptr[2] =
                      malloc(sizeof("exec ")-1
                             + buffer_string_length(host->bin_path) + 1);
                    force_assert(host->args.ptr[2]);
                    memcpy(host->args.ptr[2], "exec ", sizeof("exec ")-1);
                    memcpy(host->args.ptr[2]+sizeof("exec ")-1,
                           host->bin_path->ptr,
                           buffer_string_length(host->bin_path)+1);
                    host->args.ptr[3] = NULL;
                }

                if (host->min_procs > host->max_procs)
                    host->min_procs = host->max_procs;
                if (host->min_procs!= host->max_procs
                    && 0 != srv->srvconf.max_worker) {
                    host->min_procs = host->max_procs;
                    log_error(srv->errh, __FILE__, __LINE__,
                      "adaptive backend spawning disabled "
                      "(server.max_worker is non-zero)");
                }
                if (host->max_load_per_proc < 1)
                    host->max_load_per_proc = 0;

                if (s->debug) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "--- gw spawning local"
                      "\n\tproc: %s"
                      "\n\tport: %hu"
                      "\n\tsocket %s"
                      "\n\tmin-procs: %d"
                      "\n\tmax-procs: %d",
                      host->bin_path->ptr,
                      host->port,
                      host->unixsocket && host->unixsocket->ptr
                        ? host->unixsocket->ptr
                        : "",
                      host->min_procs,
                      host->max_procs);
                }

                for (uint32_t pno = 0; pno < host->min_procs; ++pno) {
                    gw_proc *proc = gw_proc_init();
                    proc->id = host->num_procs++;
                    host->max_id++;

                    if (buffer_string_is_empty(host->unixsocket)) {
                        proc->port = host->port + pno;
                    } else {
                        buffer_copy_buffer(proc->unixsocket, host->unixsocket);
                        buffer_append_string_len(proc->unixsocket,
                                                 CONST_STR_LEN("-"));
                        buffer_append_int(proc->unixsocket, pno);
                    }

                    if (s->debug) {
                        log_error(srv->errh, __FILE__, __LINE__,
                          "--- gw spawning"
                          "\n\tport: %hu"
                          "\n\tsocket %s"
                          "\n\tcurrent: %u / %u",
                          host->port,
                          host->unixsocket && host->unixsocket->ptr
                            ? host->unixsocket->ptr
                            : "",
                          pno, host->max_procs);
                    }

                    if (0 != gw_proc_sockaddr_init(host, proc, srv->errh)) {
                        gw_proc_free(proc);
                        goto error;
                    }

                    if (!srv->srvconf.preflight_check
                        && gw_spawn_connection(host, proc, srv->errh, s->debug)) {
                        log_error(srv->errh, __FILE__, __LINE__,
                          "[ERROR]: spawning gw failed.");
                        gw_proc_free(proc);
                        goto error;
                    }

                    gw_status_init(host, proc);

                    proc->next = host->first;
                    if (host->first) host->first->prev = proc;

                    host->first = proc;
                }
            } else {
                gw_proc *proc;

                proc = gw_proc_init();
                proc->id = host->num_procs++;
                host->max_id++;
                gw_proc_set_state(host, proc, PROC_STATE_RUNNING);

                if (buffer_string_is_empty(host->unixsocket)) {
                    proc->port = host->port;
                } else {
                    buffer_copy_buffer(proc->unixsocket, host->unixsocket);
                }

                gw_status_init(host, proc);

                host->first = proc;

                host->min_procs = 1;
                host->max_procs = 1;

                if (0 != gw_proc_sockaddr_init(host, proc, srv->errh)) goto error;
            }

            /* s->exts is list of exts -> hosts
             * s->exts now used as combined list
             *   of authorizer and responder hosts (for backend maintenance)
             * s->exts_auth is list of exts -> authorizer hosts
             * s->exts_resp is list of exts -> responder hosts
             * For each path/extension:
             * there may be an independent GW_AUTHORIZER and GW_RESPONDER
             * (The GW_AUTHORIZER and GW_RESPONDER could be handled by the same
             *  host, and an admin might want to do that for large uploads,
             *  since GW_AUTHORIZER runs prior to receiving (potentially large)
             *  request body from client and can authorizer or deny request
             *  prior to receiving the full upload)
             */
            gw_extension_insert(s->exts, &da_ext->key, host);

            if (host_mode == GW_AUTHORIZER) {
                ++host->refcount;
                gw_extension_insert(s->exts_auth, &da_ext->key, host);
            } else if (host_mode == GW_RESPONDER) {
                ++host->refcount;
                gw_extension_insert(s->exts_resp, &da_ext->key, host);
            } /*(else should have been rejected above)*/

            host = NULL;
        }
    }

    return 1;

error:
    if (NULL != host) gw_host_free(host);
    return 0;
}

int gw_get_defaults_balance(server *srv, const buffer *b) {
    if (buffer_string_is_empty(b))
        return GW_BALANCE_LEAST_CONNECTION;
    if (buffer_eq_slen(b, CONST_STR_LEN("fair")))
        return GW_BALANCE_LEAST_CONNECTION;
    if (buffer_eq_slen(b, CONST_STR_LEN("least-connection")))
        return GW_BALANCE_LEAST_CONNECTION;
    if (buffer_eq_slen(b, CONST_STR_LEN("round-robin")))
        return GW_BALANCE_RR;
    if (buffer_eq_slen(b, CONST_STR_LEN("hash")))
        return GW_BALANCE_HASH;
    if (buffer_eq_slen(b, CONST_STR_LEN("sticky")))
        return GW_BALANCE_STICKY;

    log_error(srv->errh, __FILE__, __LINE__,
      "xxxxx.balance has to be one of: "
      "least-connection, round-robin, hash, sticky, but not: %s", b->ptr);
    return GW_BALANCE_LEAST_CONNECTION;
}


static void gw_set_state(gw_handler_ctx *hctx, gw_connection_state_t state) {
    hctx->state = state;
    /*hctx->state_timestamp = log_epoch_secs;*/
}


void gw_set_transparent(gw_handler_ctx *hctx) {
    if (AF_UNIX != hctx->host->family) {
        if (-1 == fdevent_set_tcp_nodelay(hctx->fd, 1)) {
            /*(error, but not critical)*/
        }
    }
    hctx->wb_reqlen = -1;
    gw_set_state(hctx, GW_STATE_WRITE);
}


static void gw_backend_close(gw_handler_ctx * const hctx, request_st * const r) {
    if (hctx->fd >= 0) {
        fdevent_fdnode_event_del(hctx->ev, hctx->fdn);
        /*fdevent_unregister(ev, hctx->fd);*//*(handled below)*/
        fdevent_sched_close(hctx->ev, hctx->fd, 1);
        hctx->fdn = NULL;
        hctx->fd = -1;
    }

    if (hctx->host) {
        if (hctx->proc) {
            gw_proc_release(hctx->host, hctx->proc, hctx->conf.debug,
                            r->conf.errh);
            hctx->proc = NULL;
        }

        gw_host_reset(hctx->host);
        hctx->host = NULL;
    }
}

static void gw_connection_close(gw_handler_ctx * const hctx, request_st * const r) {
    gw_plugin_data *p = hctx->plugin_data;

    gw_backend_close(hctx, r);
    handler_ctx_free(hctx);
    r->plugin_ctx[p->id] = NULL;

    if (r->handler_module == p->self) {
        http_response_backend_done(r);
    }
}

static handler_t gw_reconnect(gw_handler_ctx * const hctx, request_st * const r) {
    gw_backend_close(hctx, r);

    hctx->host = gw_host_get(r,hctx->ext,hctx->conf.balance,hctx->conf.debug);
    if (NULL == hctx->host) return HANDLER_FINISHED;

    gw_host_assign(hctx->host);
    hctx->request_id = 0;
    hctx->opts.xsendfile_allow = hctx->host->xsendfile_allow;
    hctx->opts.xsendfile_docroot = hctx->host->xsendfile_docroot;
    gw_set_state(hctx, GW_STATE_INIT);
    return HANDLER_COMEBACK;
}


handler_t gw_handle_request_reset(request_st * const r, void *p_d) {
    gw_plugin_data *p = p_d;
    gw_handler_ctx *hctx = r->plugin_ctx[p->id];
    if (hctx) gw_connection_close(hctx, r);

    return HANDLER_GO_ON;
}


static void gw_conditional_tcp_fin(gw_handler_ctx * const hctx, request_st * const r) {
    /*assert(r->conf.stream_request_body & FDEVENT_STREAM_REQUEST_TCP_FIN);*/
    if (!chunkqueue_is_empty(&hctx->wb))return;
    if (!hctx->host->tcp_fin_propagate) return;
    if (hctx->gw_mode == GW_AUTHORIZER) return;
    if (r->conf.stream_request_body & FDEVENT_STREAM_REQUEST_BACKEND_SHUT_WR)
        return;

    /* propagate shutdown SHUT_WR to backend if TCP half-close on con->fd */
    r->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_BACKEND_SHUT_WR;
    r->conf.stream_request_body &= ~FDEVENT_STREAM_REQUEST_POLLIN;
    r->con->is_readable = 0;
    shutdown(hctx->fd, SHUT_WR);
    fdevent_fdnode_event_clr(hctx->ev, hctx->fdn, FDEVENT_OUT);
}

static handler_t gw_write_request(gw_handler_ctx * const hctx, request_st * const r) {
    switch(hctx->state) {
    case GW_STATE_INIT:
        /* do we have a running process for this host (max-procs) ? */
        hctx->proc = NULL;

        for (gw_proc *proc = hctx->host->first; proc; proc = proc->next) {
             if (proc->state == PROC_STATE_RUNNING) {
                 hctx->proc = proc;
                 break;
             }
        }

        /* all children are dead */
        if (hctx->proc == NULL) {
            return HANDLER_ERROR;
        }

        /* check the other procs if they have a lower load */
        for (gw_proc *proc = hctx->proc->next; proc; proc = proc->next) {
            if (proc->state != PROC_STATE_RUNNING) continue;
            if (proc->load < hctx->proc->load) hctx->proc = proc;
        }

        gw_proc_load_inc(hctx->host, hctx->proc);

        hctx->fd = fdevent_socket_nb_cloexec(hctx->host->family,SOCK_STREAM,0);
        if (-1 == hctx->fd) {
            log_error_st * const errh = r->conf.errh;
            if (errno == EMFILE || errno == EINTR) {
                log_error(errh, __FILE__, __LINE__,
                  "wait for fd at connection: %d", r->con->fd);
                return HANDLER_WAIT_FOR_FD;
            }

            log_perror(errh, __FILE__, __LINE__,
              "socket failed %d %d",r->con->srv->cur_fds,r->con->srv->max_fds);
            return HANDLER_ERROR;
        }

        ++r->con->srv->cur_fds;

        hctx->fdn = fdevent_register(hctx->ev,hctx->fd,gw_handle_fdevent,hctx);

        if (hctx->proc->is_local) {
            hctx->pid = hctx->proc->pid;
        }

        switch (gw_establish_connection(r, hctx->host, hctx->proc, hctx->pid,
                                        hctx->fd, hctx->conf.debug)) {
        case 1: /* connection is in progress */
            fdevent_fdnode_event_set(hctx->ev, hctx->fdn, FDEVENT_OUT);
            gw_set_state(hctx, GW_STATE_CONNECT_DELAYED);
            return HANDLER_WAIT_FOR_EVENT;
        case -1:/* connection error */
            return HANDLER_ERROR;
        case 0: /* everything is ok, go on */
            hctx->reconnects = 0;
            break;
        }
        __attribute_fallthrough__
    case GW_STATE_CONNECT_DELAYED:
        if (hctx->state == GW_STATE_CONNECT_DELAYED) { /*(not GW_STATE_INIT)*/
            int socket_error = fdevent_connect_status(hctx->fd);
            if (socket_error != 0) {
                gw_proc_connect_error(r, hctx->host, hctx->proc, hctx->pid,
                                      socket_error, hctx->conf.debug);
                return HANDLER_ERROR;
            }
            /* go on with preparing the request */
        }

        gw_proc_connect_success(hctx->host, hctx->proc, hctx->conf.debug, r);

        gw_set_state(hctx, GW_STATE_PREPARE_WRITE);
        __attribute_fallthrough__
    case GW_STATE_PREPARE_WRITE:
        /* ok, we have the connection */

        {
            handler_t rc = hctx->create_env(hctx);
            if (HANDLER_GO_ON != rc) {
                if (HANDLER_FINISHED != rc && HANDLER_ERROR != rc)
                    fdevent_fdnode_event_clr(hctx->ev, hctx->fdn, FDEVENT_OUT);
                return rc;
            }
        }

        /*(disable Nagle algorithm if streaming and content-length unknown)*/
        if (AF_UNIX != hctx->host->family) {
            if (r->reqbody_length < 0) {
                if (-1 == fdevent_set_tcp_nodelay(hctx->fd, 1)) {
                    /*(error, but not critical)*/
                }
            }
        }

        fdevent_fdnode_event_add(hctx->ev, hctx->fdn, FDEVENT_IN|FDEVENT_RDHUP);
        gw_set_state(hctx, GW_STATE_WRITE);
        __attribute_fallthrough__
    case GW_STATE_WRITE:
        if (!chunkqueue_is_empty(&hctx->wb)) {
            log_error_st * const errh = r->conf.errh;
          #if 0
            if (hctx->conf.debug > 1) {
                log_error(errh, __FILE__, __LINE__, "sdsx",
                  "send data to backend (fd=%d), size=%zu",
                  hctx->fd, chunkqueue_length(&hctx->wb));
            }
          #endif
            off_t bytes_out = hctx->wb.bytes_out;
            if (r->con->srv->network_backend_write(hctx->fd, &hctx->wb,
                                                   MAX_WRITE_LIMIT, errh) < 0) {
                switch(errno) {
                case EPIPE:
                case ENOTCONN:
                case ECONNRESET:
                    /* the connection got dropped after accept()
                     * we don't care about that --
                     * if you accept() it, you have to handle it.
                     */
                    log_error(errh, __FILE__, __LINE__,
                      "connection was dropped after accept() "
                      "(perhaps the gw process died), "
                      "write-offset: %lld socket: %s",
                      (long long)hctx->wb.bytes_out,
                      hctx->proc->connection_name->ptr);
                    return HANDLER_ERROR;
                default:
                    log_perror(errh, __FILE__, __LINE__, "write failed");
                    return HANDLER_ERROR;
                }
            }
            else if (hctx->wb.bytes_out > bytes_out) {
                hctx->proc->last_used = log_epoch_secs;
                if (hctx->stdin_append
                    && chunkqueue_length(&hctx->wb) < 65536 - 16384
                    && !chunkqueue_is_empty(&r->reqbody_queue)) {
                    handler_t rc = hctx->stdin_append(hctx);
                    if (HANDLER_GO_ON != rc) return rc;
                }
            }
        }

        if (hctx->wb.bytes_out == hctx->wb_reqlen) {
            fdevent_fdnode_event_clr(hctx->ev, hctx->fdn, FDEVENT_OUT);
            gw_set_state(hctx, GW_STATE_READ);
        } else {
            off_t wblen = chunkqueue_length(&hctx->wb);
            if ((hctx->wb.bytes_in < hctx->wb_reqlen || hctx->wb_reqlen < 0)
                && wblen < 65536 - 16384) {
                /*(r->conf.stream_request_body & FDEVENT_STREAM_REQUEST)*/
                if (!(r->conf.stream_request_body
                      & FDEVENT_STREAM_REQUEST_POLLIN)) {
                    r->conf.stream_request_body |=
                        FDEVENT_STREAM_REQUEST_POLLIN;
                    r->con->is_readable = 1; /* trigger optimistic client read */
                }
            }
            if (0 == wblen) {
                fdevent_fdnode_event_clr(hctx->ev, hctx->fdn, FDEVENT_OUT);
            } else {
                fdevent_fdnode_event_add(hctx->ev, hctx->fdn, FDEVENT_OUT);
            }
        }

        if (r->conf.stream_request_body
            & FDEVENT_STREAM_REQUEST_TCP_FIN)
            gw_conditional_tcp_fin(hctx, r);

        return HANDLER_WAIT_FOR_EVENT;
    case GW_STATE_READ:
        /* waiting for a response */
        return HANDLER_WAIT_FOR_EVENT;
    default:
        log_error(r->conf.errh, __FILE__, __LINE__,
          "(debug) unknown state");
        return HANDLER_ERROR;
    }
}

__attribute_cold__
static handler_t gw_write_error(gw_handler_ctx * const hctx, request_st * const r) {
    int status = r->http_status;

    if (hctx->state == GW_STATE_INIT ||
        hctx->state == GW_STATE_CONNECT_DELAYED) {

        /* (optimization to detect backend process exit while processing a
         *  large number of ready events; (this block could be removed)) */
        server * const srv = r->con->srv;
        if (0 == srv->srvconf.max_worker)
            gw_restart_dead_procs(hctx->host, srv->errh, hctx->conf.debug, 0);

        /* cleanup this request and let request handler start request again */
        if (hctx->reconnects++ < 5) return gw_reconnect(hctx, r);
    }

    if (hctx->backend_error) hctx->backend_error(hctx);
    gw_connection_close(hctx, r);
    r->http_status = (status == 400) ? 400 : 503;
    return HANDLER_FINISHED;
}

static handler_t gw_send_request(gw_handler_ctx * const hctx, request_st * const r) {
    handler_t rc = gw_write_request(hctx, r);
    return (HANDLER_ERROR != rc) ? rc : gw_write_error(hctx, r);
}


static handler_t gw_recv_response(gw_handler_ctx *hctx, request_st *r);


handler_t gw_handle_subrequest(request_st * const r, void *p_d) {
    gw_plugin_data *p = p_d;
    gw_handler_ctx *hctx = r->plugin_ctx[p->id];
    if (NULL == hctx) return HANDLER_GO_ON;

    if ((r->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN)
        && r->resp_body_started) {
        if (chunkqueue_length(&r->write_queue) > 65536 - 4096) {
            /* Note: if apps inheriting gw_handle use hctx->rb, then those apps
             * are responsible for limiting amount of data buffered in memory
             * in hctx->rb.  Currently, mod_fastcgi is the only core app doing
             * so, and the maximum FCGI_Record size is 8 + 65535 + 255 = 65798
             * (FCGI_HEADER_LEN(8)+contentLength(65535)+paddingLength(255)) */
            fdevent_fdnode_event_clr(hctx->ev, hctx->fdn, FDEVENT_IN);
        }
        else if (!(fdevent_fdnode_interest(hctx->fdn) & FDEVENT_IN)) {
            /* optimistic read from backend */
            handler_t rc;
            rc = gw_recv_response(hctx, r);          /*(might invalidate hctx)*/
            if (rc != HANDLER_GO_ON) return rc;      /*(unless HANDLER_GO_ON)*/
            fdevent_fdnode_event_add(hctx->ev, hctx->fdn, FDEVENT_IN);
        }
    }

    /* (do not receive request body before GW_AUTHORIZER has run or else
     *  the request body is discarded with handler_ctx_clear() after running
     *  the FastCGI Authorizer) */

    if (hctx->gw_mode != GW_AUTHORIZER
        && (0 == hctx->wb.bytes_in
            ? (r->state == CON_STATE_READ_POST || -1 == hctx->wb_reqlen)
            : (hctx->wb.bytes_in < hctx->wb_reqlen || hctx->wb_reqlen < 0))) {
        /* leave excess data in r->reqbody_queue, which is
         * buffered to disk if too large and backend can not keep up */
        /*(64k - 4k to attempt to avoid temporary files
         * in conjunction with FDEVENT_STREAM_REQUEST_BUFMIN)*/
        if (chunkqueue_length(&hctx->wb) > 65536 - 4096) {
            if (r->conf.stream_request_body & FDEVENT_STREAM_REQUEST_BUFMIN) {
                r->conf.stream_request_body &= ~FDEVENT_STREAM_REQUEST_POLLIN;
            }
            if (0 != hctx->wb.bytes_in) return HANDLER_WAIT_FOR_EVENT;
        }
        else {
            handler_t rc = r->con->reqbody_read(r);

            /* XXX: create configurable flag */
            /* CGI environment requires that Content-Length be set.
             * Send 411 Length Required if Content-Length missing.
             * (occurs here if client sends Transfer-Encoding: chunked
             *  and module is flagged to stream request body to backend) */
            if (-1 == r->reqbody_length && hctx->opts.backend != BACKEND_PROXY){
                return (r->conf.stream_request_body & FDEVENT_STREAM_REQUEST)
                  ? http_response_reqbody_read_error(r, 411)
                  : HANDLER_WAIT_FOR_EVENT;
            }

            if (hctx->wb_reqlen < -1 && r->reqbody_length >= 0) {
                /* (completed receiving Transfer-Encoding: chunked) */
                hctx->wb_reqlen = -hctx->wb_reqlen;
                if (hctx->stdin_append) {
                    handler_t rca = hctx->stdin_append(hctx);
                    if (HANDLER_GO_ON != rca) return rca;
                }
            }

            if ((0 != hctx->wb.bytes_in || -1 == hctx->wb_reqlen)
                && !chunkqueue_is_empty(&r->reqbody_queue)) {
                if (hctx->stdin_append) {
                    if (chunkqueue_length(&hctx->wb) < 65536 - 16384) {
                        handler_t rca = hctx->stdin_append(hctx);
                        if (HANDLER_GO_ON != rca) return rca;
                    }
                }
                else
                    chunkqueue_append_chunkqueue(&hctx->wb, &r->reqbody_queue);
                if (fdevent_fdnode_interest(hctx->fdn) & FDEVENT_OUT) {
                    return (rc == HANDLER_GO_ON) ? HANDLER_WAIT_FOR_EVENT : rc;
                }
            }
            if (rc != HANDLER_GO_ON) return rc;
        }
    }

    {
        handler_t rc =((0==hctx->wb.bytes_in || !chunkqueue_is_empty(&hctx->wb))
                       && hctx->state != GW_STATE_CONNECT_DELAYED)
          ? gw_send_request(hctx, r)
          : HANDLER_WAIT_FOR_EVENT;
        if (HANDLER_WAIT_FOR_EVENT != rc) return rc;
    }

    if (r->conf.stream_request_body & FDEVENT_STREAM_REQUEST_TCP_FIN)
        gw_conditional_tcp_fin(hctx, r);

    return HANDLER_WAIT_FOR_EVENT;
}


__attribute_cold__
static handler_t gw_recv_response_error(gw_handler_ctx * const hctx, request_st * const r, gw_proc * const proc);


static handler_t gw_recv_response(gw_handler_ctx * const hctx, request_st * const r) {
    /*(XXX: make this a configurable flag for other protocols)*/
    buffer *b = (hctx->opts.backend == BACKEND_FASTCGI
                 || hctx->opts.backend == BACKEND_AJP13)
      ? chunk_buffer_acquire()
      : hctx->response;
    const off_t bytes_in = r->write_queue.bytes_in;

    handler_t rc = http_response_read(r, &hctx->opts, b, hctx->fdn);

    if (b != hctx->response) chunk_buffer_release(b);

    gw_proc * const proc = hctx->proc;

    switch (rc) {
    default:
        /* change in r->write_queue.bytes_in used to approximate backend read,
         * since bytes read from backend, if any, might be consumed from b by
         * hctx->opts->parse callback, hampering detection here.  However, this
         * may not be triggered for partial collection of HTTP response headers
         * or partial packets for backend protocol (e.g. FastCGI) */
        if (r->write_queue.bytes_in > bytes_in)
            proc->last_used = log_epoch_secs;
        return HANDLER_GO_ON;
    case HANDLER_FINISHED:
        proc->last_used = log_epoch_secs;
        if (hctx->gw_mode == GW_AUTHORIZER
            && (200 == r->http_status || 0 == r->http_status)) {
            /*
             * If we are here in AUTHORIZER mode then a request for authorizer
             * was processed already, and status 200 has been returned. We need
             * now to handle authorized request.
             */
            char *physpath = NULL;

            gw_host * const host = hctx->host;
            if (!buffer_string_is_empty(host->docroot)) {
                buffer_copy_buffer(&r->physical.doc_root, host->docroot);
                buffer_copy_buffer(&r->physical.basedir, host->docroot);

                buffer_copy_buffer(&r->physical.path, host->docroot);
                buffer_append_path_len(&r->physical.path,
                                       CONST_BUF_LEN(&r->uri.path));
                physpath = r->physical.path.ptr;
            }

            gw_backend_close(hctx, r);
            handler_ctx_clear(hctx);

            /* don't do more than 6 loops here; normally shouldn't happen */
            if (++r->loops_per_request > 5) {
                log_error(r->conf.errh, __FILE__, __LINE__,
                  "too many loops while processing request: %s",
                  r->target_orig.ptr);
                r->http_status = 500; /* Internal Server Error */
                r->handler_module = NULL;
                return HANDLER_FINISHED;
            }

            /* restart the request so other handlers can process it */

            if (physpath) r->physical.path.ptr = NULL;
            http_response_reset(r); /*(includes r->http_status=0)*/
            /* preserve r->physical.path.ptr with modified docroot */
            if (physpath) r->physical.path.ptr = physpath;

            /*(FYI: if multiple FastCGI authorizers were to be supported,
             * next one could be started here instead of restarting request)*/

            r->handler_module = NULL;
            return HANDLER_COMEBACK;
        } else {
            /* we are done */
            gw_connection_close(hctx, r);
        }

        return HANDLER_FINISHED;
    case HANDLER_COMEBACK: /*(not expected; treat as error)*/
    case HANDLER_ERROR:
        return gw_recv_response_error(hctx, r, proc);
    }
}


__attribute_cold__
static handler_t gw_recv_response_error(gw_handler_ctx * const hctx, request_st * const r, gw_proc * const proc)
{
        /* (optimization to detect backend process exit while processing a
         *  large number of ready events; (this block could be removed)) */
        if (proc->is_local && 1 == proc->load && proc->pid == hctx->pid
            && proc->state != PROC_STATE_DIED
            && 0 == r->con->srv->srvconf.max_worker) {
            /* intentionally check proc->disabed_until before gw_proc_waitpid */
            gw_host * const host = hctx->host;
            log_error_st * const errh = r->con->srv->errh;
            if (proc->disabled_until < log_epoch_secs
                && 0 != gw_proc_waitpid(host, proc, errh)) {
                if (hctx->conf.debug) {
                    log_error(errh, __FILE__, __LINE__,
                      "--- gw spawning\n\tsocket %s\n\tcurrent: 1/%d",
                      proc->connection_name->ptr, host->num_procs);
                }

                if (gw_spawn_connection(host, proc, errh, hctx->conf.debug)) {
                    log_error(errh, __FILE__, __LINE__,
                      "respawning failed, will retry later");
                }
            }
        }

        if (r->resp_body_started == 0) {
            /* nothing has been sent out yet, try to use another child */

            if (hctx->wb.bytes_out == 0 && hctx->reconnects++ < 5) {
                log_error(r->conf.errh, __FILE__, __LINE__,
                  "response not received, request not sent on "
                  "socket: %s for %s?%.*s, reconnecting",
                  proc->connection_name->ptr,
                  r->uri.path.ptr, BUFFER_INTLEN_PTR(&r->uri.query));

                return gw_reconnect(hctx, r);
            }

            log_error(r->conf.errh, __FILE__, __LINE__,
              "response not received, request sent: %lld on "
              "socket: %s for %s?%.*s, closing connection",
              (long long)hctx->wb.bytes_out, proc->connection_name->ptr,
              r->uri.path.ptr, BUFFER_INTLEN_PTR(&r->uri.query));
        } else if (!light_btst(r->resp_htags, HTTP_HEADER_UPGRADE)) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "response already sent out, but backend returned error on "
              "socket: %s for %s?%.*s, terminating connection",
              proc->connection_name->ptr,
              r->uri.path.ptr, BUFFER_INTLEN_PTR(&r->uri.query));
        }

        if (hctx->backend_error) hctx->backend_error(hctx);
        http_response_backend_error(r);
        gw_connection_close(hctx, r);
        return HANDLER_FINISHED;
}


static handler_t gw_handle_fdevent(void *ctx, int revents) {
    gw_handler_ctx *hctx = ctx;
    request_st * const r = hctx->r;

    joblist_append(r->con);

    if (revents & FDEVENT_IN) {
        handler_t rc = gw_recv_response(hctx, r);   /*(might invalidate hctx)*/
        if (rc != HANDLER_GO_ON) return rc;         /*(unless HANDLER_GO_ON)*/
    }

    if (revents & FDEVENT_OUT) {
        return gw_send_request(hctx, r); /*(might invalidate hctx)*/
    }

    /* perhaps this issue is already handled */
    if (revents & (FDEVENT_HUP|FDEVENT_RDHUP)) {
        if (hctx->state == GW_STATE_CONNECT_DELAYED) {
            /* getoptsock will catch this one (right ?)
             *
             * if we are in connect we might get an EINPROGRESS
             * in the first call and an FDEVENT_HUP in the
             * second round
             *
             * FIXME: as it is a bit ugly.
             *
             */
            gw_send_request(hctx, r);
        } else if (r->resp_body_started) {
            /* drain any remaining data from kernel pipe buffers
             * even if (r->conf.stream_response_body
             *          & FDEVENT_STREAM_RESPONSE_BUFMIN)
             * since event loop will spin on fd FDEVENT_HUP event
             * until unregistered. */
            handler_t rc;
            const unsigned short flags = r->conf.stream_response_body;
            r->conf.stream_response_body &= ~FDEVENT_STREAM_RESPONSE_BUFMIN;
            r->conf.stream_response_body |= FDEVENT_STREAM_RESPONSE_POLLRDHUP;
            do {
                rc = gw_recv_response(hctx, r);  /*(might invalidate hctx)*/
            } while (rc == HANDLER_GO_ON);       /*(unless HANDLER_GO_ON)*/
            r->conf.stream_response_body = flags;
            return rc; /* HANDLER_FINISHED or HANDLER_ERROR */
        } else {
            gw_proc *proc = hctx->proc;
            log_error(r->conf.errh, __FILE__, __LINE__,
              "error: unexpected close of gw connection for %s?%.*s "
              "(no gw process on socket: %s ?) %d",
              r->uri.path.ptr, BUFFER_INTLEN_PTR(&r->uri.query),
              proc->connection_name->ptr, hctx->state);

            gw_connection_close(hctx, r);
        }
    } else if (revents & FDEVENT_ERR) {
        log_error(r->conf.errh, __FILE__, __LINE__,
          "gw: got a FDEVENT_ERR. Don't know why.");

        if (hctx->backend_error) hctx->backend_error(hctx);
        http_response_backend_error(r);
        gw_connection_close(hctx, r);
    }

    return HANDLER_FINISHED;
}

handler_t gw_check_extension(request_st * const r, gw_plugin_data * const p, int uri_path_handler, size_t hctx_sz) {
  #if 0 /*(caller must handle)*/
    if (NULL != r->handler_module) return HANDLER_GO_ON;
    gw_patch_connection(r, p);
    if (NULL == p->conf.exts) return HANDLER_GO_ON;
  #endif

    buffer *fn = uri_path_handler ? &r->uri.path : &r->physical.path;
    size_t s_len = buffer_string_length(fn);
    gw_extension *extension = NULL;
    gw_host *host = NULL;
    gw_handler_ctx *hctx;
    unsigned short gw_mode;

    if (0 == s_len) return HANDLER_GO_ON; /*(not expected)*/

    /* check p->conf.exts_auth list and then p->conf.ext_resp list
     * (skip p->conf.exts_auth if array is empty
     *  or if GW_AUTHORIZER already ran in this request) */
    hctx = r->plugin_ctx[p->id];
    /*(hctx not NULL if GW_AUTHORIZER ran; hctx->ext_auth check is redundant)*/
    gw_mode = (NULL == hctx || NULL == hctx->ext_auth)
      ? 0              /*GW_AUTHORIZER p->conf.exts_auth will be searched next*/
      : GW_AUTHORIZER; /*GW_RESPONDER p->conf.exts_resp will be searched next*/

    do {

        gw_exts *exts;
        if (0 == gw_mode) {
            gw_mode = GW_AUTHORIZER;
            exts = p->conf.exts_auth;
        } else {
            gw_mode = GW_RESPONDER;
            exts = p->conf.exts_resp;
        }

        if (0 == exts->used) continue;

        /* gw.map-extensions maps extensions to existing gw.server entries
         *
         * gw.map-extensions = ( ".php3" => ".php" )
         *
         * gw.server = ( ".php" => ... )
         *
         * */

        /* check if extension-mapping matches */
        if (p->conf.ext_mapping) {
            data_string *ds =
              (data_string *)array_match_key_suffix(p->conf.ext_mapping, fn);
            if (NULL != ds) {
                /* found a mapping */
                    /* check if we know the extension */
                    uint32_t k;
                    for (k = 0; k < exts->used; ++k) {
                        extension = exts->exts+k;

                        if (buffer_is_equal(&ds->value, &extension->key)) {
                            break;
                        }
                    }

                    if (k == exts->used) {
                        /* found nothing */
                        extension = NULL;
                    }
            }
        }

        if (extension == NULL) {
            size_t uri_path_len = buffer_string_length(&r->uri.path);

            /* check if extension matches */
            for (uint32_t k = 0; k < exts->used; ++k) {
                gw_extension *ext = exts->exts+k;
              #ifdef __clang_analyzer__
                force_assert(ext); /*(unnecessary; quiet clang analyzer)*/
              #endif
                size_t ct_len = buffer_string_length(&ext->key);

                /* check _url_ in the form "/gw_pattern" */
                if (ext->key.ptr[0] == '/') {
                    if (ct_len <= uri_path_len
                        && 0 == memcmp(r->uri.path.ptr, ext->key.ptr, ct_len)) {
                        extension = ext;
                        break;
                    }
                } else if (ct_len <= s_len
                           && 0 == memcmp(fn->ptr + s_len - ct_len,
                                          ext->key.ptr, ct_len)) {
                    /* check extension in the form ".fcg" */
                    extension = ext;
                    break;
                }
            }
        }

    } while (NULL == extension && gw_mode != GW_RESPONDER);

    /* extension doesn't match */
    if (NULL == extension) {
        return HANDLER_GO_ON;
    }

    /* check if we have at least one server for this extension up and running */
    host = gw_host_get(r, extension, p->conf.balance, p->conf.debug);
    if (NULL == host) {
        return HANDLER_FINISHED;
    }

    /* a note about no handler is not sent yet */
    extension->note_is_sent = 0;

    /*
     * if check-local is disabled, use the uri.path handler
     *
     */

    /* init handler-context */
    if (uri_path_handler) {
        if (host->check_local != 0) {
            return HANDLER_GO_ON;
        } else {
            /* do not split path info for authorizer */
            if (gw_mode != GW_AUTHORIZER) {
                /* the prefix is the SCRIPT_NAME,
                * everything from start to the next slash
                * this is important for check-local = "disable"
                *
                * if prefix = /admin.gw
                *
                * /admin.gw/foo/bar
                *
                * SCRIPT_NAME = /admin.gw
                * PATH_INFO   = /foo/bar
                *
                * if prefix = /cgi-bin/
                *
                * /cgi-bin/foo/bar
                *
                * SCRIPT_NAME = /cgi-bin/foo
                * PATH_INFO   = /bar
                *
                * if prefix = /, and fix-root-path-name is enable
                *
                * /cgi-bin/foo/bar
                *
                * SCRIPT_NAME = /cgi-bin/foo
                * PATH_INFO   = /bar
                *
                */
                char *pathinfo;

                /* the rewrite is only done for /prefix/? matches */
                if (host->fix_root_path_name && extension->key.ptr[0] == '/'
                                             && extension->key.ptr[1] == '\0') {
                    buffer_copy_buffer(&r->pathinfo, &r->uri.path);
                    buffer_string_set_length(&r->uri.path, 0);
                } else if (extension->key.ptr[0] == '/'
                           && buffer_string_length(&r->uri.path)
                              > buffer_string_length(&extension->key)
                           && (pathinfo =
                                 strchr(r->uri.path.ptr
                                        + buffer_string_length(&extension->key),
                                        '/')) != NULL) {
                    /* rewrite uri.path and pathinfo */

                    buffer_copy_string(&r->pathinfo, pathinfo);
                    buffer_string_set_length(
                      &r->uri.path,
                      buffer_string_length(&r->uri.path)
                      - buffer_string_length(&r->pathinfo));
                }
            }
        }
    }

    if (!hctx) hctx = handler_ctx_init(hctx_sz);

    hctx->ev               = r->con->srv->ev;
    hctx->r                = r;
    hctx->plugin_data      = p;
    hctx->host             = host;
    hctx->proc             = NULL;
    hctx->ext              = extension;
    gw_host_assign(host);

    hctx->gw_mode = gw_mode;
    if (gw_mode == GW_AUTHORIZER) {
        hctx->ext_auth = hctx->ext;
    }

    /*hctx->conf.exts        = p->conf.exts;*/
    /*hctx->conf.exts_auth   = p->conf.exts_auth;*/
    /*hctx->conf.exts_resp   = p->conf.exts_resp;*/
    /*hctx->conf.ext_mapping = p->conf.ext_mapping;*/
    hctx->conf.balance     = p->conf.balance;
    hctx->conf.proto       = p->conf.proto;
    hctx->conf.debug       = p->conf.debug;

    hctx->opts.fdfmt = S_IFSOCK;
    hctx->opts.authorizer = (gw_mode == GW_AUTHORIZER);
    hctx->opts.local_redir = 0;
    hctx->opts.xsendfile_allow = host->xsendfile_allow;
    hctx->opts.xsendfile_docroot = host->xsendfile_docroot;

    r->plugin_ctx[p->id] = hctx;

    r->handler_module = p->self;

    if (r->conf.log_request_handling) {
        log_error(r->conf.errh, __FILE__, __LINE__, "handling it in mod_gw");
    }

    return HANDLER_GO_ON;
}

static void gw_handle_trigger_host(gw_host * const host, log_error_st * const errh, const int debug) {
    /*
     * TODO:
     *
     * - add timeout for a connect to a non-gw process
     *   (use state_timestamp + state)
     *
     * perhaps we should kill a connect attempt after 10-15 seconds
     *
     * currently we wait for the TCP timeout which is 180 seconds on Linux
     */

    /* check each child proc to detect if proc exited */

    gw_proc *proc;
    time_t idle_timestamp;
    int overload = 1;

    for (proc = host->first; proc; proc = proc->next) {
        gw_proc_waitpid(host, proc, errh);
    }

    gw_restart_dead_procs(host, errh, debug, 1);

    /* check if adaptive spawning enabled */
    if (host->min_procs == host->max_procs) return;
    if (buffer_string_is_empty(host->bin_path)) return;

    for (proc = host->first; proc; proc = proc->next) {
        if (proc->load <= host->max_load_per_proc) {
            overload = 0;
            break;
        }
    }

    if (overload && host->num_procs && host->num_procs < host->max_procs) {
        /* overload, spawn new child */
        if (debug) {
            log_error(errh, __FILE__, __LINE__,
              "overload detected, spawning a new child");
        }

        gw_proc_spawn(host, errh, debug);
    }

    idle_timestamp = log_epoch_secs - host->idle_timeout;
    for (proc = host->first; proc; proc = proc->next) {
        if (host->num_procs <= host->min_procs) break;
        if (0 != proc->load) continue;
        if (proc->pid <= 0) continue;
        if (proc->last_used >= idle_timestamp) continue;

        /* terminate proc that has been idling for a long time */
        if (debug) {
            log_error(errh, __FILE__, __LINE__,
              "idle-timeout reached, terminating child: socket: %s pid %d",
              proc->unixsocket ? proc->unixsocket->ptr : "", proc->pid);
        }

        gw_proc_kill(host, proc);

        /* proc is now in unused, let next second handle next process */
        break;
    }

    for (proc = host->unused_procs; proc; proc = proc->next) {
        gw_proc_waitpid(host, proc, errh);
    }
}

static void gw_handle_trigger_exts(gw_exts * const exts, log_error_st * const errh, const int debug) {
    for (uint32_t j = 0; j < exts->used; ++j) {
        gw_extension *ex = exts->exts+j;
        for (uint32_t n = 0; n < ex->used; ++n) {
            gw_handle_trigger_host(ex->hosts[n], errh, debug);
        }
    }
}

static void gw_handle_trigger_exts_wkr(gw_exts *exts, log_error_st *errh) {
    for (uint32_t j = 0; j < exts->used; ++j) {
        gw_extension * const ex = exts->exts+j;
        for (uint32_t n = 0; n < ex->used; ++n) {
            gw_host * const host = ex->hosts[n];
            for (gw_proc *proc = host->first; proc; proc = proc->next) {
                if (proc->state == PROC_STATE_OVERLOADED)
                    gw_proc_check_enable(host, proc, errh);
            }
        }
    }
}

handler_t gw_handle_trigger(server *srv, void *p_d) {
    gw_plugin_data * const p = p_d;
    int wkr = (0 != srv->srvconf.max_worker && p->srv_pid != srv->pid);
    log_error_st * const errh = srv->errh;
    int global_debug = 0;

    if (NULL == p->cvlist) return HANDLER_GO_ON;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        gw_plugin_config *conf = NULL;
        int debug = global_debug;
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* xxxxx.server */
                if (cpv->vtype == T_CONFIG_LOCAL) conf = cpv->v.v;
                break;
              case 2: /* xxxxx.debug */
                debug = (int)cpv->v.u;
                if (0 == i) global_debug = (int)cpv->v.u;
              default:
                break;
            }
        }

        if (NULL == conf || NULL == conf->exts) continue;

        /* (debug flag is only active if set in same scope as xxxxx.server
         *  or global scope (for convenience))
         * (unable to use p->defaults.debug since gw_plugin_config
         *  might be part of a larger plugin_config) */
        wkr
          ? gw_handle_trigger_exts_wkr(conf->exts, errh)
          : gw_handle_trigger_exts(conf->exts, errh, debug);
    }

    return HANDLER_GO_ON;
}

handler_t gw_handle_waitpid_cb(server *srv, void *p_d, pid_t pid, int status) {
    gw_plugin_data * const p = p_d;
    if (0 != srv->srvconf.max_worker && p->srv_pid != srv->pid)
        return HANDLER_GO_ON;
    log_error_st * const errh = srv->errh;
    int global_debug = 0;

    if (NULL == p->cvlist) return HANDLER_GO_ON;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        gw_plugin_config *conf = NULL;
        int debug = global_debug;
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* xxxxx.server */
                if (cpv->vtype == T_CONFIG_LOCAL) conf = cpv->v.v;
                break;
              case 2: /* xxxxx.debug */
                debug = (int)cpv->v.u;
                if (0 == i) global_debug = (int)cpv->v.u;
              default:
                break;
            }
        }

        if (NULL == conf || NULL == conf->exts) continue;

        /* (debug flag is only active if set in same scope as xxxxx.server
         *  or global scope (for convenience))
         * (unable to use p->defaults.debug since gw_plugin_config
         *  might be part of a larger plugin_config) */
        const time_t cur_ts = log_epoch_secs;
        gw_exts *exts = conf->exts;
        for (uint32_t j = 0; j < exts->used; ++j) {
            gw_extension *ex = exts->exts+j;
            for (uint32_t n = 0; n < ex->used; ++n) {
                gw_host *host = ex->hosts[n];
                gw_proc *proc;
                for (proc = host->first; proc; proc = proc->next) {
                    if (!proc->is_local || proc->pid != pid) continue;

                    gw_proc_waitpid_log(host, proc, errh, status);
                    gw_proc_set_state(host, proc, PROC_STATE_DIED);
                    proc->pid = 0;

                    /* restart, but avoid spinning if child exits too quickly */
                    if (proc->disabled_until < cur_ts) {
                        if (proc->state != PROC_STATE_KILLED)
                            proc->disabled_until = cur_ts;
                        if (gw_spawn_connection(host, proc, errh, debug)) {
                            log_error(errh, __FILE__, __LINE__,
                              "ERROR: spawning gw failed.");
                        }
                    }

                    return HANDLER_FINISHED;
                }
                for (proc = host->unused_procs; proc; proc = proc->next) {
                    if (!proc->is_local || proc->pid != pid) continue;

                    gw_proc_waitpid_log(host, proc, errh, status);
                    if (proc->state != PROC_STATE_KILLED)
                        proc->disabled_until = cur_ts;
                    gw_proc_set_state(host, proc, PROC_STATE_DIED);
                    proc->pid = 0;
                    return HANDLER_FINISHED;
                }
            }
        }
    }

    return HANDLER_GO_ON;
}
