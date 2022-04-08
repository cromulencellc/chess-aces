/*
 * Part of Very Secure FTPd
 * Licence: GPL v2
 * Author: Chris Evans
 * login.c
 *
 * Code for the handling the common login functions needed across
 * both the one and two process security model.
 */
#include "login.h"
#include "parseconf.h"
#include "postlogin.h"
#include "postprivparent.h"
#include "privsock.h"
#include "seccompsandbox.h"
#include "secutil.h"
#include "session.h"
#include "str.h"
#include "ssl.h"
#include "sysdeputil.h"
#include "sysstr.h"
#include "sysutil.h"
#include "tunables.h"
#include "utility.h"

static void handle_sigchld(void* duff);

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

void
common_do_login(struct vsf_session* p_sess, const struct mystr* p_user_str,
                int do_chroot, int anon)
{
  int was_anon = anon;
  const struct mystr* p_orig_user_str = p_user_str;
  int newpid;
  vsf_sysutil_install_null_sighandler(kVSFSysUtilSigCHLD);
  /* Tells the pre-login child all is OK (it may exit in response) */
  priv_sock_send_result(p_sess->parent_fd, PRIV_SOCK_RESULT_OK);
  if (!p_sess->control_use_ssl)
  {
    (void) vsf_sysutil_wait();
  }
  else
  {
    p_sess->ssl_slave_active = 1;
  }
  /* Handle loading per-user config options */
  handle_per_user_config(p_user_str);
  /* Set this before we fork */
  p_sess->is_anonymous = anon;
  priv_sock_close(p_sess);
  priv_sock_init(p_sess);
  vsf_sysutil_install_sighandler(kVSFSysUtilSigCHLD, handle_sigchld, 0, 1);
  if (tunable_isolate_network && !tunable_port_promiscuous)
  {
    newpid = vsf_sysutil_fork_newnet();
  }
  else
  {
    newpid = vsf_sysutil_fork();
  }
  if (newpid == 0)
  {
    struct mystr guest_user_str = INIT_MYSTR;
    struct mystr chroot_str = INIT_MYSTR;
    struct mystr chdir_str = INIT_MYSTR;
    struct mystr userdir_str = INIT_MYSTR;
    unsigned int secutil_option = VSF_SECUTIL_OPTION_USE_GROUPS |
                                  VSF_SECUTIL_OPTION_NO_PROCS;
    /* Child - drop privs and start proper FTP! */
    /* This PR_SET_PDEATHSIG doesn't work for all possible process tree setups.
     * The other cases are taken care of by a shutdown() of the command
     * connection in our SIGTERM handler.
     */
    vsf_set_die_if_parent_dies();
    priv_sock_set_child_context(p_sess);
    if (tunable_guest_enable && !anon)
    {
      p_sess->is_guest = 1;
      /* Remap to the guest user */
      if (tunable_guest_username)
      {
        str_alloc_text(&guest_user_str, tunable_guest_username);
      }
      p_user_str = &guest_user_str;
      if (!tunable_virtual_use_local_privs)
      {
        anon = 1;
        do_chroot = 1;
      }
    }
    if (do_chroot)
    {
      secutil_option |= VSF_SECUTIL_OPTION_CHROOT;
    }
    if (!anon)
    {
      secutil_option |= VSF_SECUTIL_OPTION_CHANGE_EUID;
    }
    if (!was_anon && tunable_allow_writeable_chroot)
    {
      secutil_option |= VSF_SECUTIL_OPTION_ALLOW_WRITEABLE_ROOT;
    }
    vsf_sysutil_calculate_chdir_dir(was_anon, &userdir_str, &chroot_str, &chdir_str,
                        p_user_str, p_orig_user_str);
    vsf_secutil_change_credentials(p_user_str, &userdir_str, &chroot_str,
                                   0, secutil_option);
    if (!str_isempty(&chdir_str))
    {
      (void) str_chdir(&chdir_str);
    }
    str_free(&guest_user_str);
    str_free(&chroot_str);
    str_free(&chdir_str);
    str_free(&userdir_str);
    p_sess->is_anonymous = anon;
    seccomp_sandbox_init();
    seccomp_sandbox_setup_postlogin(p_sess);
    seccomp_sandbox_lockdown();
    process_post_login(p_sess);
    bug("should not get here: common_do_login");
  }
  /* Parent */
  priv_sock_set_parent_context(p_sess);
  if (tunable_ssl_enable)
  {
    ssl_comm_channel_set_producer_context(p_sess);
  }
  /* The seccomp sandbox lockdown for the priv parent is done inside here */
  vsf_priv_parent_postlogin(p_sess);
  bug("should not get here in common_do_login");
}

void
handle_per_user_config(const struct mystr* p_user_str)
{
  struct mystr filename_str = INIT_MYSTR;
  struct vsf_sysutil_statbuf* p_statbuf = 0;
  struct str_locate_result loc_result;
  int retval;
  if (!tunable_user_config_dir)
  {
    return;
  }
  /* Security paranoia - ignore if user has a / in it. */
  loc_result = str_locate_char(p_user_str, '/');
  if (loc_result.found)
  {
    return;
  }
  str_alloc_text(&filename_str, tunable_user_config_dir);
  str_append_char(&filename_str, '/');
  str_append_str(&filename_str, p_user_str);
  retval = str_stat(&filename_str, &p_statbuf);
  if (!vsf_sysutil_retval_is_error(retval))
  {
    /* Security - file ownership check now in vsf_parseconf_load_file() */
    vsf_parseconf_load_file(str_getbuf(&filename_str), 1);
  }
  else if (vsf_sysutil_get_error() != kVSFSysUtilErrNOENT)
  {
    die("error opening per-user config file");
  }
  str_free(&filename_str);
  vsf_sysutil_free(p_statbuf);
}
