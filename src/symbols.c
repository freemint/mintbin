/* symbols.c - symbol management for MiNTbin.
   Copyright (C) 1999 Guido Flohr <gufl0000@stud.uni-sb.de>.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef STDC_HEADERS
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#else
extern int errno;
#endif

#include <error.h>

#include <mint/a.out.h>
#include <mint/prg-out.h>

#include <libmb.h>

#include "system.h"

#include "targets.h"
#include "symbols.h"

#define SIZEOF_STRUCT_NLIST 12
#define SIZEOF_STRUCT_ASYM  14

#define INITIAL_HTAB_SIZE 1024

static int swap_gnu_symbols_in PARAMS ((struct mintbin_target*));
static int swap_dri_symbols_in PARAMS ((struct mintbin_target*, int));

struct nlist*
#if PROTOTYPES
lookup_symbol (struct mintbin_target* target, const char* name,
	       size_t* min_idx)
#else
lookup_symbol (target, name, min_idx)
     struct mintbin_target* target;
     const char* name;
     size_t *min_idx;
#endif
{
  size_t i;

  if (target == NULL)
    return NULL;
  if (target->symtab == NULL && swap_symtab_in (target, 0) != 0)
    return NULL;

  for (i = *min_idx; i < target->symcount; i++)
    if (strcmp (name, target->symtab[i].n_un.n_name) == 0)
      {
	*min_idx = i + 1;
	return (target->symtab + i);
      }

  *min_idx = target->symcount;
  return NULL;
}

int
#if PROTOTYPES
swap_symtab_in (struct mintbin_target* target, int preserve)
#else
swap_symtab_in (target, preserve)
     struct mintbin_target* target;
     int preserve
#endif
{
  if (target == NULL)
    return -1;

  if (target->format == ar_library_target
      || target->format == oldstyle_ar_library_target)
    {
      error (EXIT_SUCCESS, 0, _("\
%s: cannot lookup symbols in archives"),
	     target->filename);
      return -1;
    }

  if (target->execp.g_syms == 0)
    {
      error (EXIT_SUCCESS, 0, _("%s: no symbol table in file"),
	     target->filename);
      return -1;
    }

  if (lseek (target->desc, N_SYMOFF (target->execp),
	     SEEK_SET) != N_SYMOFF (target->execp)) 
    {
      error (EXIT_SUCCESS, errno, _("%s: seek error"),
	     target->filename);
      return -1;
    }

  switch (target->format)
    {
    case aout_target:
    case prg_target:
      if (target->execp.g_symbol_format == _GNU_FORMAT)
	return swap_gnu_symbols_in (target);
      /* Else fall thru'.  */
    case oldstyle_prg_target:
      if (target->execp.g_symbol_format != _DRI_FORMAT)
        {
	  error (EXIT_SUCCESS, 0, _("\
%s: format of symbol table not recognized"),
		 target->filename);
	  return -1;
	}
      return swap_dri_symbols_in (target, preserve);
    default:
      break;
    }

  return -1;
}

static int
#if PROTOTYPES
swap_gnu_symbols_in (struct mintbin_target* target)
#else
swap_gnu_symbols_in (target)
     struct mintbin_target* target;
#endif
{
  error (EXIT_SUCCESS, 0, _("\
%s: cannot read GNU symbol table yet"), target->filename);
  return -1;
}

/* Read a DRI symbol table from input file.  The file pointer
   is already at the right position.  If PRESERVE is non-zero then
   put the original type verbatim into the n_desc member of struct
   nlist.  */
static int
#if PROTOTYPES
swap_dri_symbols_in (struct mintbin_target* target, int preserve)
#else
swap_dri_symbols_in (target, preserve)
     struct mintbin_target* target;
     int preserve;
#endif
{
  /* We don't know how many symbols we finally have.  But we
     can calculate the maximum.  This will temporarily waste
     memory in most cases but it is preferable to a lot of
     realloc operations.  SLOTS is the number of struct asym
     entries in the file and this is the maximum number of
     symbols we can possibly encounter.  */
  size_t slots = target->execp.g_syms / SIZEOF_STRUCT_ASYM;
  size_t number_of_symbols = 0;
  size_t i;  /* Guess what!  */

  /* We maintain one pointer for reading and one for writing
     our data.  So we can reuse READBUF (because it will
     always be large enough to hold all strings, even with
     null-termination byte.  */
  unsigned char* readcrs;
  unsigned char* writecrs;

  /* We slurp in the entire symbol table at once.  This will
     waste a little memory.  If this is a concern we could
     later realloc to a smaller size and adjust the n_name
     pointers for all symbols.  One could even allocate
     less memory from the very beginning (idea:  if we have
     for example 100 slots we need a minimum of 100 * 9 bytes
     for the names if we have only short names or a maximum
     of 50 * 23 = 1150 bytes if all names are long.  Hm,
     I rather waste a mere 20 percent instead of getting annoyed
     by a lot of overflow and then being forced to read
     over and over.  */
  readcrs = writecrs = target->strtab = xmalloc (target->execp.g_syms);
  target->symtab = xmalloc (slots * sizeof (struct nlist));
  memset (target->symtab, 0, slots * sizeof (struct nlist));

  errno = 0;
  if (read (target->desc, (void*) readcrs, target->execp.g_syms)
      != target->execp.g_syms)
    {
      if (errno != 0)
	error (EXIT_SUCCESS, errno, _("%s: read error"),
	       target->filename);
      else
	error (EXIT_SUCCESS, 0, _("%s: file truncated"),
	       target->filename);
      return -1;
    }

  for (i = 0; i < slots; i++)
    {
      unsigned short a_type;
      int long_name = 0;
      struct nlist* sym = target->symtab + number_of_symbols++;
      int j;

      a_type = get16 (readcrs + 8);
      sym->n_value = get32 (readcrs + 10);

      /* If this symbol has a GST compatible long name remember
	 it and mask out the appropriate bits.  */

      if ((a_type & A_LNAM) != 0)
	{
	  long_name = 1;
	  a_type &= ~A_LNAM;
	}

      if (preserve)
	sym->n_desc = a_type;

      /* First the unique types.  */
      if (a_type == A_TFILE)
	{
	  sym->n_type = N_FN;
	  a_type = 0;
	}
      else
	{
	  if ((a_type & A_GLOBL) != 0)
	    {
	      sym->n_type |= N_EXT;
	      a_type &= ~A_GLOBL;
	    }
	  
	  /* Guess a type for the symbol.  In case of a conflict A_EQU
	     will precede A_TEXT, A_DATA, and A_BSS.  */
	  if ((a_type & A_EQU) != 0)
	    sym->n_type |= N_ABS;
	  else if ((a_type & A_TEXT) != 0)
	    sym->n_type |= N_TEXT;
	  else if ((a_type & A_DATA) != 0)
	    sym->n_type |= N_DATA;
	  else if ((a_type & A_BSS) != 0)
	    sym->n_type |= N_BSS;
	}
      /* Move the name to the adequate position.  We cannot use
	 memcpy here because the blocks will often overlap.  */
      memmove (writecrs, readcrs, 8);
      sym->n_un.n_name = writecrs;
      for (j = 0; j < 8; j++)
	{
	  if (readcrs[j] == '\0')
	    break;
        }
      readcrs += SIZEOF_STRUCT_ASYM;
      writecrs += j;

      /* Read the possible rest of the symbol name.  */
      if (long_name)
	{
	  i++;
	  if (j < 8)
	    error (EXIT_SUCCESS, 0, _("\
%s: warning: null-byte in symbol-name ``%s'' encountered"),
		   target->filename, sym->n_un.n_name);
          if (i < slots)
	    {
	      memmove (writecrs, readcrs, SIZEOF_STRUCT_ASYM);

	      for (j = 0; j < SIZEOF_STRUCT_ASYM; j++)
		{
		  if (writecrs[j] == '\0')
		    break;
		}
	      readcrs += SIZEOF_STRUCT_ASYM;
	      writecrs += j;
            }
	  else
	    {
	      error (EXIT_SUCCESS, 0, ("%s: warning: file truncated"),
		     target->filename);
	    }
	}
      /* Null-terminate string.  */
      *writecrs++ = '\0';
    }

  target->symcount = number_of_symbols;

  return 0;
}



