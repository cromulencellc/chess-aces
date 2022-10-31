/*
 * ProFTPD - mod_sftp miscellaneous
 * Copyright (c) 2010-2020 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#ifndef MOD_SFTP_MISC_H
#define MOD_SFTP_MISC_H

#include "mod_sftp.h"

int sftp_misc_chown_file(pool *, pr_fh_t *);
int sftp_misc_chown_path(pool *, const char *);
const char *sftp_misc_get_chroot(pool *);
int sftp_misc_namelist_contains(pool *, const char *, const char *);
const char *sftp_misc_namelist_shared(pool *, const char *, const char *);
char *sftp_misc_vroot_abs_path(pool *, const char *, int);

#endif /* MOD_SFTP_MISC_H */
