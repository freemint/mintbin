/* tempname.c -- Create a temporary file name.
   Copyright (C) 1991, 92, 93, 94, 95, 1997 Free Software Foundation, Inc.

   This file is part of MiNTBin.

   MiNTBin is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   MiNTBin is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiNTBin; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* $Id$ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# ifdef HAVE_STRING_H
#  include <string.h>
# else
#  ifdef HAVE_STRINGS_H
#   include <strings.h>
#  endif
# endif
#endif

#include <sys/stat.h>

#ifdef HAVE_TIME_H
# include <time.h>		/* ctime, maybe time_t */
#endif

#include <libmb.h>

/* Return the name of a temporary file in the same directory as FILENAME.  */

char *
make_tempname (filename)
     char *filename;
{
  static char template[] = "stXXXXXX";
  char *tmpname;
  char *slash = strrchr (filename, '/');

#if defined (__DJGPP__) || defined (__GO32__) || defined (_WIN32) || defined (__MINT__)
  if (slash == NULL)
    slash = strrchr (filename, '\\');
#endif

  if (slash != (char *) NULL)
    {
      char c;

      c = *slash;
      *slash = 0;
      tmpname = xmalloc (strlen (filename) + sizeof (template) + 1);
      strcpy (tmpname, filename);
      strcat (tmpname, "/");
      strcat (tmpname, template);
      mktemp (tmpname);
      *slash = c;
    }
  else
    {
      tmpname = xmalloc (sizeof (template));
      strcpy (tmpname, template);
      mktemp (tmpname);
    }
  return tmpname;
}
