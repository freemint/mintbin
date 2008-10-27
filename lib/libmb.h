/* libmb.h - Utilities for MiNTbin.
   Copyright (C) 1998 Guido Flohr (gufl0000@stud.uni-sb.de).

This file is part of MiNTbin.

MiNTbin is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

MiNTbin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MiNTbin; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* $Id$ */

#ifndef _LIBMB_H
# define _LIBMB_H 1  /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "system.h"

/* Prototypes for utility library functions.  */
extern char* xstrdup PARAMS ((const char*));
extern void* xmalloc PARAMS ((size_t));

#endif /* _LIBMB_H */
