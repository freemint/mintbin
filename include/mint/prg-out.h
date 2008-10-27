/* prg-out.h - Definitions and declarations for MiNT executables
   in extended format as produced by the GNU linker 2.9.1 and
   later.
   Written by Guido Flohr (gufl0000@stud.uni-sb.de).

   This file is in the public domain.  */

/* $Id$ */

#ifndef __MINT_PRG_OUT_H__
# define __MINT_PRG_OUT_H__  /* Allow multiple inclusion.  */

#include <mint/a.out.h>      /* Grab macro definitions.  */

#ifndef __exec_unsigned
# ifdef __MSHORT
#  define __exec_unsigned unsigned long
# else
#  define __exec_unsigned unsigned
# endif
#endif

#define PRG_NMAGIC     0x601a
#define PRG_OMAGIC     0xdead

#define PRG_MINT_MAGIC 0x4d694e54

struct prg_exec {
  unsigned short g_branch;      /* PRG_NMAGIC for NMAGIC or ZMAGIC
                                   executables, PRG_OMAGIC for
				   relocatable linker output.  */
  unsigned long  g_text;        /* Length of text section from the OS point
				   of view.  This is always 228 bytes more
				   than the a_text member says.  */
  unsigned long  g_data;        /* Length of data section.  */
  unsigned long  g_bss;         /* Length of bss section.  */
  unsigned long  g_syms;        /* Length of symbol table.  */
  unsigned long  g_extmagic;    /* Always PRG_MINT_MAGIC.
				   (in ASCII: ``MiNT'').  */
  unsigned long  g_flags;       /* Atari special flags.  */
  unsigned long  g_abs;         /* Non-zero if absolute (no relocation
                                   info).  Should therefore always be
				   zero.  */
  
  /* We extend this header now to provide the information that the
     binutils want to see.  Everything following will actually
     be part of the text segment (from MiNT's point of view).  
     As a consequence the text section has 228 bytes of 
     redundancy. 

     The following eight bytes contain assembler instructions for
     kernels that don't evaluate e_entry.  The assembler code
     will perform a far jump to the entry point.  Simply treat
     it as opaque in your own programs.  */
  unsigned char g_jump_entry[8];

  /* Now following a standard a.out header.  Note that the values
     may differ from the one given on top.  The traditional header
     contains the values that the OS wants to see, the values below
     are the values that make the binutils work.  */
  unsigned long a_info;          /* Magic number and stuff.  */
  __exec_unsigned a_text;        /* Length of text section in bytes.  */
  __exec_unsigned a_data;        /* Length of data section.  */
  __exec_unsigned a_bss;         /* Length of standard symbol table.  */
  __exec_unsigned a_syms;        /* Length of symbol table.  */
  __exec_unsigned a_entry;       /* Start address.  */
  __exec_unsigned a_trsize;      /* Length of text relocation info.  */
  __exec_unsigned a_drsize;      /* Length of data relocation info.  */

  unsigned long g_tparel_pos;    /* File position of TPA relative
			            relocation info.  */
  unsigned long g_tparel_size;   /* Length of TPA relative relocation
			            info.  */

  /* This is for extensions.  */
  unsigned long g_stkpos;        /* If stacksize is hardcoded into
                                    the executable you will find it
			            at file offset g_stkpos.  If
				    not this is NULL.  */

  unsigned long g_symbol_format; /* Format of the symbol table.  Non-zero
				    for non-standard symbol formats (i.e.
                                    not struct nlist and string table).   */
                                     
  unsigned char g_pad0[172];     /* Unused; should be zero.  */
};

/* Executable flags.  */
#define _MINT_F_FASTLOAD	0x01	/* Don't clear heap.  */
#define _MINT_F_ALTLOAD	        0x02    /* OK to load in alternate RAM.  */
#define _MINT_F_ALTALLOC	0x04	/* OK to malloc from alt. RAM.  */
#define _MINT_F_BESTFIT	        0x08	/* Stack in BSS.  */
#define _MINT_F_RESERVED F_BESTFIT
#define _MINT_F_MEMFLAGS	0xf0	/* Masks out protection bits.  */
#define _MINT_F_SHTEXT         0x800    /* Program's text may be shared.  */

/* Protection bits.  */
#define _MINT_F_MEMPROTECTION   0xf0    /* Masks out protection bits.  */    
#define _MINT_F_MEMPRIVATE      0x00    /* Memory is private.  */
#define _MINT_F_MEMGLOBAL       0x10    /* Read/write access to memory 
                                           allowed.  */
#define _MINT_F_MEMSUPER        0x20    /* Only supervisor access allowed.  */
#define _MINT_F_MEMREADABLE     0x30    /* Any read access OK.  */

/* Base offset for shared text executables.  */
#define BASE_OFFSET	(-32768)

/* There is no macro for CMAGIC = 0x601a.  This conflicts with the
   core file magic in a.out.h.  */

#define	ISRELOCINFO	0	/* relocation information is present */
				/*  any other value - no reloc info  */

#define A_BADMAG(x) \
((((x).g_branch != PRG_NMAGIC) && ((x).g_branch != PRG_OMAGIC)) \
 || ((x).g_extmagic != PRG_MINT_MAGIC) \
 || (N_BADMAG (x)))

#define A_TXTOFF(x) N_TXTOFF(x)
#define A_DATOFF(x) N_DATOFF(x)
#define A_SYMOFF(x) N_SYMOFF(x)
#define A_STROFF(x) N_STROFF(x)

/* Format of a DRI symbol table entry (deprecated).  */
struct asym {
  char           a_name[8];  /* Symbol name.  */
  unsigned short a_type;     /* Type flag, i.e. A_TEXT etc; see below.  */
  unsigned long  a_value;    /* Value of this symbol (or sdb offset).  */
};

/* Extended DRI symbol table entry (deprecated).  */
struct xsym {
  char           a_name[8];  /* Symbol name (first part).  */
  unsigned short a_type;     
  unsigned long  a_value;
  char           a_morename[14];  /* If a_type & A_LN rest of name.  */
};

/* Type flags for struct asym/xsym.  This is really stupid.  No problem
   to define a symbol that belongs to the BSS as well as to the text
   section _and_ the data section.  Who invented this?  */
#define A_UNDF   0x0000      /* Undefined.  */
#define A_BSS    0x0100      /* BSS.  */
#define A_TEXT   0x0200      /* Text segment.  */
#define A_DATA   0x0400      /* Data segment.  */
#define A_EXT    0x0800      /* External.  */
#define A_EQREG  0x1000      /* Equated register.  */
#define A_GLOBL  0x2000      /* Global.  */
#define A_EQU    0x4000      /* Equated (absolute with GNU binutils).  */
#define A_DEF    0x8000      /* Defined.  */

#define A_LNAM   0x0048      /* GST compatible long name.  If the a_type
				member of struct asym/xsym && A_LNAM is
				non-zero it is a long name.  A maximum
				of 14 bytes follow a_value.  */

#define A_TFILE 0x0280       /* Text file name (object module).  */
#define A_TFARC 0x02C0       /* Text file archive.  This clashes with
				A_LNAM.  */

#endif /* __MINT_PRG_OUT_H */









