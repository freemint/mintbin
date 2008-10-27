/* targets.h - All known binary targets for MiNTbin.
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

#ifndef _TARGETS_H
# define _TARGETS_H 1  /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

/* Note:  In your own programs you should always include
   <a.out.h> and not <mint/a.out.h>.  Here we want to make
   bomb-proof that the a.out.h supplied with the package
   gets included and not the system header file.  */
#include <mint/a.out.h>

#include "exec.h"

#include "system.h"

/* Utility macro.  Get a 16- or 32 bit value from a pointer to
   unsigned char.  */
#define get16(c) (((c)[0] << 8) | ((c)[1])) 
#define get32(c) \
(((c)[0] << 24) \
 | ((c)[1] << 16) \
 | ((c)[2] << 8) \
 | ((c)[3]))

enum mintbin_targets_enum {
  /* Invalid target file.  */
#define invalid_target invalid_target
  invalid_target,
  
  /* Standard a.out object files.  */
#define aout_target aout_target
  aout_target,

  /* Standard ar libraries.  */
#define ar_library_target ar_library_target
  ar_library_target,

  /* Old style ar libraries (``GNU is not eUnuchs'').  */
#define oldstyle_ar_library_target oldstyle_ar_library_target
  oldstyle_ar_library_target,

  /* MiNT prg executables in extended format.  */
#define prg_target prg_target
  prg_target,

  /* Old-style prg executables or other 0x601a files that are
     not recognized.  */
#define oldstyle_prg_target oldstyle_prg_target
  oldstyle_prg_target
};

struct mintbin_target {
  /* One of the above described target types.  */
  enum mintbin_targets_enum format;

  /* Literal name of the target.  The pointer is a mere copy of
     the pointer passed to the constructor function.  */
  const char* name;

  /* Filename of the target file.  */
  const char* filename;

  /* Associated file descriptor.  */
  int desc;

  /* File size in bytes.  */
  size_t size;

  /* Pointer to struct exec. */
  struct internal_exec execp;

  /* Non-zero if we already attempted to look up _stksize.  */
  int    stksize_looked_up;

  /* Pointer to original header data.  */
  unsigned char* header;

  /* Number of valid bytes in HEADER.  */
  int header_bytes;

  /* If non-NULL this points to the symbol table.  */
  struct nlist* symtab;

  /* If non-NULL this points to the string table.  */
  char* strtab;

  /* Number of symbols in symbol table.  */
  size_t symcount;
};

/* Old-style ar header. */
#define OLD_ARMAG "Gnu is Not eUnuchs.\012"
#define OLD_SARMAG    20
#define OLD_ARFMAG "\015\012"
struct oldstyle_ar_hdr 
{
  char ar_name[16];
  char ar_size[12];
  char ar_date[12];
  char ar_mode[8];	
  char ar_uid[4];	
  char ar_gid[4];	
  char ar_fmag[2];	
};

/* Open file NAME with FLAGS as per open.  */
struct mintbin_target* open_target PARAMS ((const char* name,
					    int flags));

/* Close TARGET and free all associated resources.  */
int close_target PARAMS ((struct mintbin_target* target));

#endif /* _TARGETS_H */



