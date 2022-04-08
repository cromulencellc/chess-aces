#ifndef VSF_LOGIN_H
#define VSF_LOGIN_H

struct mystr;
struct vsf_session* p_sess;

/* common_do_login()
 * PURPOSE
 * Handle the common login procedures necessary for one and two
 * process run types
 * PARAMETERS
 * p_sess       - the current FTP session object
 * p_usr_str    - buffer object containing the user string
 * do_chroot    - value indicating the need to chroot
 * anon         - value indicating that the login is anonymous
 * RETURNS
 * Does not return a value
 */
void common_do_login(struct vsf_session* p_sess,
                            const struct mystr* p_user_str, int do_chroot,
                            int anon);

/* handle_per_user_config()
 * PURPOSE
 * Handle the configuration on a per user basis
 * PARAMETERS
 * p_usr_str    - buffer object containing the user string
 * RETURNS
 * Does not return a value
 */
void handle_per_user_config(const struct mystr* p_user_str);

#endif /* VSF_LOGIN_H */