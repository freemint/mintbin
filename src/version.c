/* version.c -- version information for MiNTbin.
   Copyright (C) 1998, 1999 Guido Flohr <gufl0000@stud.uni-sb.de>.

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

#include <stdio.h>

#include "system.h"

/* This is the version numbers for the binutils.  They all change in
   lockstep -- it's easier that way. */

const char *program_version = VERSION;

/* Print the version number and copyright information, and exit.  This
   implements the --version option for the various programs.  */

void
print_version (name)
     const char *name;
{
  /* This output is intended to follow the GNU standards document.  */
  printf (_("%s (%s - FreeMiNT binary utilities) %s\n"),
	  name, PACKAGE, program_version);
  /* Note to translators:  Please do either not translate the following
     copyright notice at all or at least include the original english
     version!  */
  printf (_("Copyright (C) 1999 Guido Flohr (gufl0000@stud.uni-sb.de)\n"));
  printf (_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License.  This program has absolutely no warranty.\n"));
  exit (0);
}



