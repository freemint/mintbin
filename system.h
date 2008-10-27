/* system.h - Standard definitions and utility macros for MiNTbin.
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

/* $Header$ */

#ifndef _MINTBIN_SYSTEM_H
# define _MINTBIN_SYSTEM_H 1  /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef PARAMS
# if defined(__STDC__) || defined (__GNUC__)
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

#if STDC_HEADERS
# include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#include <libintl.h>
#define _(String) gettext(String)

#ifndef N_
# define N_(String) (String)
#endif

/* src/version.c */
extern void print_version PARAMS ((const char *));

#endif /* _MINTBIN_SYSTEM_H */










