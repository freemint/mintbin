/* exec.h - Internal representation of a struct exec.
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

#ifndef _EXEC_H
# define _EXEC_H 1  /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <mint/a.out.h>
#include <mint/prg-out.h>

#define _GNU_FORMAT 0
#define _DRI_FORMAT 1

/* Internal definition of a struct exec.  This fits for object
   files and for executables.  */
struct internal_exec {
  unsigned long  a_info;          /* Magic number and stuff.  */
  unsigned long  a_text;          /* Length of text section in bytes.  */
  unsigned long  a_data;          /* Length of data section.  */
  unsigned long  a_bss;           /* Length of standard symbol 
				     table.  */
  unsigned long  a_syms;          /* Length of symbol table.  */
  unsigned long  a_entry;         /* Start address.  */
  unsigned long  a_trsize;        /* Length of text relocation
                                     info.  */
  unsigned long  a_drsize;        /* Length of data relocation
				     info.  */

  unsigned long  g_text;          /* Length of text section for OS.  */
  unsigned long  g_syms;          /* Length of symbol table (incl.
				     string table) for OS.  */
  unsigned long  g_str;           /* Length of string table.  */
  unsigned long  g_flags;         /* Atari special flags.  */
  unsigned long  g_abs;           /* Non-zero if absolute (no relocation
                                     info.  */
  unsigned long  g_tparel_pos;    /* File position of base relative
				     relocation info.  */
  unsigned long  g_tparel_size;   /* Length of base relative relocation
				     info.  */
  unsigned long  g_stkpos;        /* If stacksize is hardcoded into
                                     the executable you will find it
				     at file offset g_stkpos.  If
				     not this is NULL.  */
  unsigned long  g_symbol_format; /* Format of the symbol table.  See
                                     definitions for _*_FORMAT
                                     above.  */
};

#endif /* _EXEC_H */










