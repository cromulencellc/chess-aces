/* $MirOS: src/gnu/usr.bin/cvs/lib/getdate.h,v 1.7 2016/10/22 20:15:03 tg Exp $ */

/* Parse a string into an internal time stamp.

   Copyright (C) 1995, 1997, 1998, 2003, 2004, 2005
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef GETDATE_H
#define GETDATE_H

#include <stdbool.h>
#ifndef IN_RCS
#include "timespec.h"
#else
#include <sys/time.h>
#include <time.h>
#endif

bool get_date (struct timespec *, char const *, struct timespec const *);

#endif /* GETDATE_H */
