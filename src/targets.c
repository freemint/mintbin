/* targets.c -- handle binary target files.
   Copyright (C) 1998, 1999 Guido Flohr <gufl0000@stud.uni-sb.de>.

This file is part of MiNTBin.

MiNTBin is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

MiNTBin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MiNTBin; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* $Id$ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# include <stdio.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else 
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#else
extern int errno;
#endif

#include <sys/stat.h>

#include <error.h>

#include <mint/prg-out.h>
#include <a.out.h>
#include <ar.h>

#include <libmb.h>

#include "targets.h"

#include "system.h"


static int swap_prg_header_in PARAMS ((struct mintbin_target*));
static int swap_aout_header_in PARAMS ((struct mintbin_target*));
static int swap_ar_header_in PARAMS ((struct mintbin_target*));
static int swap_old_ar_header_in PARAMS ((struct mintbin_target*));

struct mintbin_target*
#if PROTOTYPES
open_target (const char* name, int flags)
#else
open_target (name, flags)
     const char* name;
     int flags;
#endif
{
  struct mintbin_target* target = xmalloc (sizeof (*target));
  int read_bytes = 0;
  struct stat statbuf;

  memset (target, 0, sizeof (struct mintbin_target));

  target->filename = name;
  if (stat (name, &statbuf) != 0)
    {
      error (EXIT_SUCCESS, errno, _("%s: stat error"),
	     target->filename);
      goto error_return;
    }
  target->size = statbuf.st_size;

  target->desc = open (name, flags);
  if (target->desc < 0) 
    {
      error (EXIT_SUCCESS, errno, _("%s: open error"),
	     target->filename);
      goto error_return;
    }
  
  /* Now determine file type.  We always try to read a maximum of 
     256 bytes.  */
  target->header = xmalloc (256);
  read_bytes = read (target->desc, (void*) target->header, 256);

  if (read_bytes < 28)
    {
      error (EXIT_SUCCESS, errno, _("read error on ``%s''"),
	     target->filename);
      free (target->header);
      return NULL;
    }

  target->header_bytes = read_bytes;

  switch (target->header[0]) {
  case 0x60:  /* Executables in extended MiNT format.  */
  case 0xde:  /* Relocatable linker output in extended MiNT format.  */
    if (swap_prg_header_in (target) != 0)
      goto error_return;
    break;
  case 0x00:
    if (swap_aout_header_in (target) != 0)
      goto error_return;
    break;    /* a.out object file.  */
  case '!':  /* a.out (maybe b.out) archive.  */
    if (swap_ar_header_in (target) != 0)
      goto error_return;
    break;
  case 'G':  /* Gnu is Not eUnuchs old style archive.  */
    if (swap_old_ar_header_in (target) != 0)
      goto error_return;
    break;
  default:
    error (EXIT_SUCCESS, 0, _("%s: file format not recognized"),
	   target->filename);
    goto error_return;
  }
  return target;
  
 error_return:
  (void) close_target (target);
  return NULL;
}

int
#if PROTOTYPES
close_target (struct mintbin_target* target)
#else
close_target (target)
     struct mintbin_target* target;
#endif
{
  int retval = -1;

  if (target != NULL) 
    {
      if (target->desc >= 0)
	retval = close (target->desc);
      target->desc = -1;
      if (target->header != NULL)
	free (target->header);
      target->header = NULL;
      if (target->symtab != NULL)
	free (target->symtab);
      target->symtab = NULL;
      if (target->strtab != NULL)
	free (target->strtab);
      target->strtab = NULL;
      free (target);
      target = NULL;
    }

  return retval;
}

static int
#if PROTOTYPES 
swap_prg_header_in (struct mintbin_target* target)
#else
swap_prg_header_in (target)
     struct mintbin_target* target;
#endif
{
  int is_601a = 1;
  int is_extended_mint = 0;
  unsigned char* crs = target->header;
  
  target->format = invalid_target;

  if (crs[0] == 0xde && crs[1] == 0xad)
    is_601a = 0;
  else if (crs[0] != 0x60 || crs[1] != 0x1a)
    {
      error (EXIT_SUCCESS, 0, _("\
%s: file format not recognized"), target->filename);
      return -1;
    }

  /* This technique may seem awkward but it will work on
     64 bit hosts.  */
  crs += 2;

  target->execp.g_text = get32 (crs);
  crs += 4;
  target->execp.a_data = get32 (crs);
  crs += 4;
  target->execp.a_bss = get32 (crs);
  crs += 4;
  target->execp.g_syms = get32 (crs);
  crs += 4;
  if (crs[0] == 'M' || crs[1] == 'i'
      || crs[2] == 'N' || crs[3] == 'T') 
    {
      is_extended_mint = 1;
    }
  crs += 4;
  target->execp.g_flags = get32 (crs);
  crs += 4;
  target->execp.g_abs = get16 (crs);
  if (!is_extended_mint) {
    N_SET_INFO (target->execp, PRG_NMAGIC, 0, 0);
    target->format = oldstyle_prg_target;
    target->execp.a_text = target->execp.g_text;
    target->execp.g_tparel_pos = 0x1c /* Size of old-style header.  */
      + target->execp.g_text + target->execp.a_data
      + target->execp.g_syms;
    target->execp.g_symbol_format = 1;
    return 0;
  }

  if (target->header_bytes < 256)
    {
      error (EXIT_SUCCESS, 0, _("%s: file truncated"), target->filename);
      return -1;
    }

  crs += 10;  /* Skip 2nd branch instruction and header length.  */
  target->execp.a_info = get32 (crs);
  crs += 4;
  target->execp.a_text = get32 (crs);
  if (target->execp.a_text + 228 != target->execp.g_text)
    {
      error (EXIT_SUCCESS, 0, _("%s: inconsistent text segment size in header"),
	     target->filename);
      return -1;
    }

  crs += 4;
  if (target->execp.a_data != get32 (crs))
    {
      error (EXIT_SUCCESS, 0, _("%s: inconsistent data segment size in header"),
	     target->filename);
      return -1;
    }
  crs += 4;
  if (target->execp.a_bss != get32 (crs))
    {
      error (EXIT_SUCCESS, 0, _("%s: inconsistent bss size in header"),
	     target->filename);
      return -1;
    }
  crs += 4;
  target->execp.a_syms = get32 (crs);
  crs += 4;
  target->execp.a_entry = get32 (crs);
  crs += 4;
  target->execp.a_trsize = get32 (crs);
  if (target->execp.a_trsize != 0)
    if (is_601a || N_MAGIC (target->execp) != OMAGIC)
      {
        error (EXIT_SUCCESS, 0, _("\
%s: error: relocation against text segment found in program image"),
	       target->filename);
	return -1;
      }
  crs += 4;
  target->execp.a_drsize = get32 (crs);
  if (target->execp.a_drsize != 0)
    if (is_601a || N_MAGIC (target->execp) != OMAGIC)
      {
        error (EXIT_SUCCESS, 0, _("\
%s: error: relocation against text segment found in program image"),
	       target->filename);
	return -1;
      }
  crs += 4;

  target->execp.g_tparel_pos = get32 (crs);
  crs += 4;
  target->execp.g_tparel_size = get32 (crs);
  crs += 4;
  target->execp.g_stkpos = get32 (crs);
  crs += 4;
  target->stksize_looked_up = 1;

  target->execp.g_symbol_format = get32 (crs);
  
  target->format = prg_target;

  return 0;
}

static int
#if PROTOTYPES 
swap_aout_header_in (struct mintbin_target* target)
#else
swap_aout_header_in (target)
     struct mintbin_target* target;
#endif
{
  unsigned char* crs = target->header;
  
  target->format = invalid_target;

  if (target->header_bytes < N_TXTOFF (target->execp))
    {
      error (EXIT_SUCCESS, 0, _("%s: file truncated"), target->filename);
      return -1;
    }
  else
    target->header_bytes = N_TXTOFF (target->execp);

  /* This technique may seem awkward but it will work on
     64 bit hosts.  */
  target->execp.a_info = get32 (crs);
  crs += 4;
  target->execp.a_text = get32 (crs);
  crs += 4;
  target->execp.a_data = get32 (crs);
  crs += 4;
  target->execp.a_bss = get32 (crs);
  crs += 4;
  target->execp.a_syms = get32 (crs);
  crs += 4;
  target->execp.a_entry = get32 (crs);
  crs += 4;
  target->execp.a_trsize = get32 (crs);
  crs += 4;
  target->execp.a_drsize = get32 (crs);
  crs += 4;

  if (N_BADMAG (target->execp))
    {
      error (EXIT_SUCCESS, 0, _("%s: bad magic number"), target->filename);
      return -1;
    }

  /* Fill execp with default values.  */
  target->execp.g_text = target->execp.a_text;
  target->execp.g_syms = target->execp.a_syms;
  
  target->format = aout_target;
  return 0;
}

static int
#if PROTOTYPES 
swap_ar_header_in (struct mintbin_target* target)
#else
swap_ar_header_in (target)
     struct mintbin_target* target;
#endif
{
  target->format = invalid_target;

  if (target->header_bytes < SARMAG)
    {
      error (EXIT_SUCCESS, 0,  _("%s: file truncated"), target->filename);
      return -1;
    }
  target->header_bytes = SARMAG;

  if (memcmp (target->header, ARMAG, SARMAG) != 0)
    {
      error (EXIT_SUCCESS, 0, _("\
%s: file format not recognized"), target->filename);
      return -1; 
    }

  target->format = ar_library_target;
  return 0;
}

static int
#if PROTOTYPES 
swap_old_ar_header_in (struct mintbin_target* target)
#else
swap_old_ar_header_in (target)
     struct mintbin_target* target;
#endif
{
  target->format = invalid_target;

  if (target->header_bytes < OLD_SARMAG)
    {
      error (EXIT_SUCCESS, 0, _("%s: file truncated"), target->filename);
      return -1;
    }
  target->header_bytes = OLD_SARMAG;

  if (memcmp (target->header, OLD_ARMAG, OLD_SARMAG) != 0)
    {
      error (EXIT_SUCCESS, 0, _("\
%s: file format not recognized"), target->filename);
      return -1; 
    }

  target->format = oldstyle_ar_library_target;
  return 0;
}









