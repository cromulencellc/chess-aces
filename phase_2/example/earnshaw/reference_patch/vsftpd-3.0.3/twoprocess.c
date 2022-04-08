/*
 * Part of Very Secure FTPd
 * License: GPL v2
 * Author: Chris Evans
 * twoprocess.c
 *
 * Code implementing the standard, secure two process security model.
 */

#include "twoprocess.h"
#include "privops.h"
#include "prelogin.h"
#include "postlogin.h"
#include "postprivparent.h"
#include "session.h"
#include "privsock.h"
#include "secutil.h"
#include "filestr.h"
#include "login.h"
#include "str.h"
#include "sysstr.h"
#include "utility.h"
#include "tunables.h"
#include "defs.h"
#include "parseconf.h"
#include "ssl.h"
#include "readwrite.h"
#include "sysutil.h"
#include "sysdeputil.h"
#include "sslslave.h"
#include "seccompsandbox.h"

static void drop_all_privs(void);
static void handle_sigchld(void* duff);
static void handle_sigterm(void* duff);
static void process_login_req(struct vsf_session* p_sess);

static void
handle_sigchld(void* duff)
{

  struct vsf_sysutil_wait_retval wait_retval = vsf_sysutil_wait();
  (void) duff;
  /* Child died, so we'll do the same! Report it as an error unless the child
   * exited normally with zero exit code
   */
  if (vsf_sysutil_retval_is_error(vsf_sysutil_wait_get_retval(&wait_retval)))
  {
    die("waiting for child");
  }
  else if (!vsf_sysutil_wait_exited_normally(&wait_retval))
  {
    die("child died");
  }
  vsf_sysutil_exit(0);
}

static void
handle_sigterm(void* duff)
{
  (void) duff;
  /* Blow away the connection to make sure no process lingers. */
  vsf_sysutil_shutdown_failok(VSFTP_COMMAND_FD);
  /* Will call the registered exit function to clean up u/wtmp if needed. */
  vsf_sysutil_exit(1);
}

void
vsf_two_process_start(struct vsf_session* p_sess)
{
  vsf_sysutil_install_sighandler(kVSFSysUtilSigTERM, handle_sigterm, 0, 1);
  /* Overrides the SIGKILL setting set by the standalone listener. */
  vsf_set_term_if_parent_dies();
  /* Create the comms channel between privileged parent and no-priv child */
  priv_sock_init(p_sess);
  if (tunable_ssl_enable)
  {
    /* Create the comms channel between the no-priv SSL child and the low-priv
     * protocol handling child.
     */
    ssl_comm_channel_init(p_sess);
  }
  vsf_sysutil_install_sighandler(kVSFSysUtilSigCHLD, handle_sigchld, 0, 1);
  {
    int newpid;
    if (tunable_isolate_network)
    {
      newpid = vsf_sysutil_fork_newnet();
    }
    else
    {
      newpid = vsf_sysutil_fork();
    }
    if (newpid != 0)
    {
      priv_sock_set_parent_context(p_sess);
      if (tunable_ssl_enable)
      {
        ssl_comm_channel_set_consumer_context(p_sess);
      }
      /* Parent - go into pre-login parent process mode */
      while (1)
      {
        process_login_req(p_sess);
      }
    }
  }
  /* Child process - time to lose as much privilege as possible and do the
   * login processing
   */
  vsf_set_die_if_parent_dies();
  priv_sock_set_child_context(p_sess);
  if (tunable_ssl_enable)
  {
    ssl_comm_channel_set_producer_context(p_sess);
  }
  if (tunable_local_enable && tunable_userlist_enable)
  {
    int retval = -1;
    if (tunable_userlist_file)
    {
      retval = str_fileread(&p_sess->userlist_str, tunable_userlist_file,
                            VSFTP_CONF_FILE_MAX);
    }
    if (vsf_sysutil_retval_is_error(retval))
    {
      die2("cannot read user list file:", tunable_userlist_file);
    }
  }
  drop_all_privs();
  seccomp_sandbox_init();
  seccomp_sandbox_setup_prelogin(p_sess);
  seccomp_sandbox_lockdown();
  init_connection(p_sess);
  /* NOTREACHED */
}

static void
drop_all_privs(void)
{
  struct mystr user_str = INIT_MYSTR;
  struct mystr dir_str = INIT_MYSTR;
  unsigned int option = VSF_SECUTIL_OPTION_CHROOT | VSF_SECUTIL_OPTION_NO_PROCS;
  if (!tunable_ssl_enable)
  {
    /* Unfortunately, can only enable this if we can be sure of not using SSL.
     * In the SSL case, we'll need to receive data transfer file descriptors.
     */
    option |= VSF_SECUTIL_OPTION_NO_FDS;
  }
  if (tunable_nopriv_user)
  {
    str_alloc_text(&user_str, tunable_nopriv_user);
  }
  if (tunable_secure_chroot_dir)
  {
    str_alloc_text(&dir_str, tunable_secure_chroot_dir);
  }
  /* Be kind: give good error message if the secure dir is missing */
  {
    struct vsf_sysutil_statbuf* p_statbuf = 0;
    if (vsf_sysutil_retval_is_error(str_lstat(&dir_str, &p_statbuf)))
    {
      die2("vsftpd: not found: directory given in 'secure_chroot_dir':",
           tunable_secure_chroot_dir);
    }
    vsf_sysutil_free(p_statbuf);
  }
  vsf_secutil_change_credentials(&user_str, &dir_str, 0, 0, option);
  str_free(&user_str);
  str_free(&dir_str);
}

void
vsf_two_process_login(struct vsf_session* p_sess,
                      const struct mystr* p_pass_str)
{
  char result;
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_LOGIN);
  priv_sock_send_str(p_sess->child_fd, &p_sess->user_str);
  priv_sock_send_str(p_sess->child_fd, p_pass_str);
  priv_sock_send_int(p_sess->child_fd, p_sess->control_use_ssl);
  priv_sock_send_int(p_sess->child_fd, p_sess->data_use_ssl);
  result = priv_sock_get_result(p_sess->child_fd);
  if (result == PRIV_SOCK_RESULT_OK)
  {
    /* Miracle. We don't emit the success message here. That is left to
     * process_post_login().
     * Exit normally, unless we are remaining as the SSL read / write child.
     */
    if (!p_sess->control_use_ssl)
    {
      vsf_sysutil_exit(0);
    }
    else
    {
      ssl_slave(p_sess);
    }
    /* NOTREACHED */
  }
  else if (result == PRIV_SOCK_RESULT_BAD)
  {
    /* Continue the processing loop.. */
    return;
  }
  else
  {
    die("priv_sock_get_result");
  }
}

int
vsf_two_process_get_priv_data_sock(struct vsf_session* p_sess)
{
  char res;
  unsigned short port = vsf_sysutil_sockaddr_get_port(p_sess->p_port_sockaddr);
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_GET_DATA_SOCK);
  priv_sock_send_int(p_sess->child_fd, port);
  res = priv_sock_get_result(p_sess->child_fd);
  if (res == PRIV_SOCK_RESULT_BAD)
  {
    return -1;
  }
  else if (res != PRIV_SOCK_RESULT_OK)
  {
    die("could not get privileged socket");
  }
  return priv_sock_recv_fd(p_sess->child_fd);
}

void
vsf_two_process_pasv_cleanup(struct vsf_session* p_sess)
{
  char res;
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_PASV_CLEANUP);
  res = priv_sock_get_result(p_sess->child_fd);
  if (res != PRIV_SOCK_RESULT_OK)
  {
    die("could not clean up socket");
  }
}

int
vsf_two_process_pasv_active(struct vsf_session* p_sess)
{
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_PASV_ACTIVE);
  return priv_sock_get_int(p_sess->child_fd);
}

unsigned short
vsf_two_process_listen(struct vsf_session* p_sess)
{
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_PASV_LISTEN);
  return (unsigned short) priv_sock_get_int(p_sess->child_fd);
}

int
vsf_two_process_get_pasv_fd(struct vsf_session* p_sess)
{
  char res;
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_PASV_ACCEPT);
  res = priv_sock_get_result(p_sess->child_fd);
  if (res == PRIV_SOCK_RESULT_BAD)
  {
    return priv_sock_get_int(p_sess->child_fd);
  }
  else if (res != PRIV_SOCK_RESULT_OK)
  {
    die("could not accept on listening socket");
  }
  return priv_sock_recv_fd(p_sess->child_fd);
}

void
vsf_two_process_chown_upload(struct vsf_session* p_sess, int fd)
{
  char res;
  priv_sock_send_cmd(p_sess->child_fd, PRIV_SOCK_CHOWN);
  priv_sock_send_fd(p_sess->child_fd, fd);
  res = priv_sock_get_result(p_sess->child_fd);
  if (res != PRIV_SOCK_RESULT_OK)
  {
    die("unexpected failure in vsf_two_process_chown_upload");
  }
}

static void
process_login_req(struct vsf_session* p_sess)
{
  enum EVSFPrivopLoginResult e_login_result = kVSFLoginNull;
  char cmd;
  /* Blocks */
  cmd = priv_sock_get_cmd(p_sess->parent_fd);
  if (cmd != PRIV_SOCK_LOGIN)
  {
    die("bad request");
  }
  /* Get username and password - we must distrust these */
  {
    struct mystr password_str = INIT_MYSTR;
    priv_sock_get_str(p_sess->parent_fd, &p_sess->user_str);
    priv_sock_get_str(p_sess->parent_fd, &password_str);
    p_sess->control_use_ssl = priv_sock_get_int(p_sess->parent_fd);
    p_sess->data_use_ssl = priv_sock_get_int(p_sess->parent_fd);
    if (!tunable_ssl_enable)
    {
      p_sess->control_use_ssl = 0;
      p_sess->data_use_ssl = 0;
    }
    e_login_result = vsf_privop_do_login(p_sess, &password_str);
    str_free(&password_str);
  }
  switch (e_login_result)
  {
    case kVSFLoginFail:
      priv_sock_send_result(p_sess->parent_fd, PRIV_SOCK_RESULT_BAD);
      return;
      break;
    case kVSFLoginAnon:
      str_free(&p_sess->user_str);
      if (tunable_ftp_username)
      {
        str_alloc_text(&p_sess->user_str, tunable_ftp_username);
      }
      common_do_login(p_sess, &p_sess->user_str, 1, 1);
      break;
    case kVSFLoginReal:
      {
        int do_chroot = 0;
        if (tunable_chroot_local_user)
        {
          do_chroot = 1;
        }
        if (tunable_chroot_list_enable)
        {
          struct mystr chroot_list_file = INIT_MYSTR;
          int retval = -1;
          if (tunable_chroot_list_file)
          {
            retval = str_fileread(&chroot_list_file, tunable_chroot_list_file,
                                  VSFTP_CONF_FILE_MAX);
          }
          if (vsf_sysutil_retval_is_error(retval))
          {
            die2("could not read chroot() list file:",
                 tunable_chroot_list_file);
          }
          if (str_contains_line(&chroot_list_file, &p_sess->user_str))
          {
            if (do_chroot)
            {
              do_chroot = 0;
            }
            else
            {
              do_chroot = 1;
            }
          }
          str_free(&chroot_list_file);
        }
        common_do_login(p_sess, &p_sess->user_str, do_chroot, 0);
      }
      break;
    case kVSFLoginNull:
      /* Fall through */
    default:
      bug("weird state in process_login_request");
      break;
  }
  /* NOTREACHED */
}