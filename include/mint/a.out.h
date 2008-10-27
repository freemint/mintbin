/* a.out.h - Definitions and declarations for GNU-style a.out
   binaries.
   Written by Guido Flohr (gufl0000@stud.uni-sb.de).

   This file is in the public domain.  */

/* $Id$ */

#ifndef __A_OUT_GNU_H__
# define __A_OUT_GNU_H__  /* Allow multiple inclusion.  */

#define __GNU_EXEC_MACROS__

#ifndef __exec_unsigned
# ifdef __MSHORT
#  define __exec_unsigned unsigned long
# else
#  define __exec_unsigned unsigned
# endif
#endif

#ifndef __STRUCT_EXEC_OVERRIDE__
struct exec
{
  __exec_unsigned long a_info;		/* Use macros N_MAGIC, etc for 
					   access.  */
  __exec_unsigned a_text;		/* Length of text, in bytes */
  __exec_unsigned a_data;		/* Length of data, in bytes */
  __exec_unsigned a_bss;		/* Length of uninitialized data area 
					   for file, in bytes.  */
  __exec_unsigned a_syms;		/* Length of symbol table data in file,
					   in bytes.  */
  __exec_unsigned a_entry;		/* Start address.  */
  __exec_unsigned a_trsize;		/* Length of relocation info for text,
					   in bytes.  */
  __exec_unsigned a_drsize;		/* Length of relocation info for data,
					   in bytes.  */
};
#endif /* not __STRUCT_EXEC_OVERRIDE__ */

#define N_TRSIZE(a)	((a).a_trsize)
#define N_DRSIZE(a)	((a).a_drsize)
#define N_SYMSIZE(a)	((a).a_syms)

/* These go in the N_MACHTYPE field.  Most of them are not needed
   for MiNT but having them defined here may be handy when compiling
   software written for other platforms.  */
enum machine_type {
#if defined (M_OLDSUN2)
  M__OLDSUN2 = M_OLDSUN2,
#else
  M_OLDSUN2 = 0,
#endif
#if defined (M_68010)
  M__68010 = M_68010,
#else
  M_68010 = 1,
#endif
#if defined (M_68020)
  M__68020 = M_68020,
#else
  M_68020 = 2,
#endif
#if defined (M_SPARC)
  M__SPARC = M_SPARC,
#else
  M_SPARC = 3,
#endif
  /* skip a bunch so we don't run into any of suns numbers */
  /* make these up for the ns32k*/
  M_NS32032 = (64),		/* ns32032 running ? */
  M_NS32532 = (64 + 5),		/* ns32532 running mach */

  M_386 = 100,
  M_29K = 101,          /* AMD 29000 */
  M_386_DYNIX = 102,	/* Sequent running dynix */
  M_ARM = 103,		/* Advanced Risc Machines ARM */
  M_SPARCLET = 131,	/* SPARClet = M_SPARC + 128 */
  M_386_NETBSD = 134,	/* NetBSD/i386 binary */
  M_68K_NETBSD = 135,	/* NetBSD/m68k binary */
  M_68K4K_NETBSD = 136,	/* NetBSD/m68k4k binary */
  M_532_NETBSD = 137,	/* NetBSD/ns32k binary */
  M_SPARC_NETBSD = 138,	/* NetBSD/sparc binary */
  M_PMAX_NETBSD = 139,	/* NetBSD/pmax (MIPS little-endian) binary */
  M_VAX_NETBSD = 140,	/* NetBSD/vax binary */
  M_ALPHA_NETBSD = 141,	/* NetBSD/alpha binary */
  M_SPARCLET_1 = 147,	/* 0x93, reserved */
  M_MIPS1 = 151,        /* MIPS R2000/R3000 binary */
  M_MIPS2 = 152,        /* MIPS R4000/R6000 binary */
  M_SPARCLET_2 = 163,	/* 0xa3, reserved */
  M_SPARCLET_3 = 179,	/* 0xb3, reserved */
  M_SPARCLET_4 = 195,	/* 0xc3, reserved */
  M_HP200 = 200,	/* HP 200 (68010) BSD binary */
  M_HP300 = (300 % 256), /* HP 300 (68020+68881) BSD binary */
  M_HPUX = (0x20c % 256), /* HP 200/300 HPUX binary */
  M_SPARCLET_5 = 211,	/* 0xd3, reserved */
  M_SPARCLET_6 = 227,	/* 0xe3, reserved */
  M_SPARCLET_7 = 243	/* 0xf3, reserved */
};

#define N_DYNAMIC(exec) ((exec).a_info & 0x80000000)

#ifndef N_MAGIC
# define N_MAGIC(exec) ((exec).a_info & 0xffff)
#endif

#define N_MACHTYPE(exec) ((enum machine_type) (((exec.a_info) >> 16) & 0xff))

#define N_FLAGS(exec) (((exec).a_info >> 24) & 0xff)

#define N_SET_INFO(exec, magic, type, flags) \
((exec).a_info = ((magic) & 0xffff) \
 | (((int) (type) & 0xff) << 16) \
 | (((flags) & 0xff) << 24))

#define N_SET_DYNAMIC(exec, dynamic) \
((exec).a_info = (dynamic) ? ((exec).a_info | 0x80000000) : \
((exec).a_info & 0x7fffffff))

#define N_SET_MAGIC(exec, magic) \
((exec).a_info = (((exec).a_info & 0xffff0000) | ((magic) & 0xffff)))

#define N_SET_MACHTYPE(exec, machtype) \
((exec).a_info = \
 ((exec).a_info&0xff00ffff) | ((((int)(machtype))&0xff) << 16))

#define N_SET_FLAGS(exec, flags) \
((exec).a_info = \
 ((exec).a_info&0x00ffffff) | (((flags) & 0xff) << 24))

/* Code indicating object file or impure executable.  */
#define OMAGIC 0407
/* Code indicating pure executable.  */
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */
#define ZMAGIC 0413
/* This indicates a demand-paged executable with the header in the text. 
   The first page is unmapped to help trap NULL pointer references.  They
   are a Linux invention and not used in MiNT.  */
#define QMAGIC 0314

/* Code indicating core file.  */
#define CMAGIC 0421

#if !defined (N_BADMAG)
#define N_BADMAG(x)	  (N_MAGIC (x) != OMAGIC		\
			&& N_MAGIC (x) != NMAGIC		\
  			&& N_MAGIC (x) != ZMAGIC \
		        && N_MAGIC (x) != QMAGIC)
#endif

#define _N_HDROFF(x) 0

#if !defined (N_TXTOFF)
     /* The last case (neither ZMAGIC, nor NMAGIC, nor OMAGIC)
	helps to simplify code for old-style executables.  Just
	don't call N_BADMAG, set a_info to some invalid value
	(for example 0x601a) and you can use most of the other
	macros defined in this file.  */
# define N_TXTOFF(x) \
  ((N_MAGIC (x) == ZMAGIC || N_MAGIC (x) == NMAGIC) ? \
   256 : (N_MAGIC(x) == OMAGIC \
	  ? sizeof (struct exec) : 0x1c))
#endif

#if !defined (N_DATOFF)
# define N_DATOFF(x) (N_TXTOFF (x) + (x).a_text)
#endif

#if !defined (N_TRELOFF)
# define N_TRELOFF(x) (N_DATOFF (x) + (x).a_data)
#endif

#if !defined (N_DRELOFF)
# define N_DRELOFF(x) (N_TRELOFF (x) + N_TRSIZE (x))
#endif

#if !defined (N_SYMOFF)
# define N_SYMOFF(x) (N_DRELOFF (x) + N_DRSIZE (x))
#endif

#if !defined (N_STROFF)
# define N_STROFF(x) (N_SYMOFF (x) + N_SYMSIZE (x))
#endif

/* Address of text segment in memory after it is loaded.  */
#if !defined (N_TXTADDR)
#define N_TXTADDR(x) \
     (((N_MAGIC(x) == ZMAGIC) || (N_MAGIC (x) == NMAGIC)) \
      ? 256 : sizeof (struct exec))
#endif

#define _N_SEGMENT_ROUND(x) 1

#define _N_TXTENDADDR(x) (N_TXTADDR (x) + (x).a_text)

#ifndef N_DATADDR
# define N_DATADDR(x) \
     (N_MAGIC(x) == OMAGIC ? (_N_TXTENDADDR (x)) \
      : (_N_SEGMENT_ROUND (_N_TXTENDADDR (x))))
#endif

/* Address of bss segment in memory after it is loaded.  */
#if !defined (N_BSSADDR)
#define N_BSSADDR(x) (N_DATADDR(x) + (x).a_data)
#endif

#if !defined (N_NLIST_DECLARED)
struct nlist {
  union {
    char *n_name;
    struct nlist *n_next;
    long n_strx;
  } n_un;
  unsigned char n_type;
  char n_other;
  short n_desc;
  unsigned long n_value;
};
#endif /* no N_NLIST_DECLARED.  */

#if !defined (N_UNDF)
#define N_UNDF 0
#endif
#if !defined (N_ABS)
#define N_ABS 2
#endif
#if !defined (N_TEXT)
#define N_TEXT 4
#endif
#if !defined (N_DATA)
#define N_DATA 6
#endif
#if !defined (N_BSS)
#define N_BSS 8
#endif
#if !defined (N_FN)
#define N_FN 15
#endif
#if !defined (N_COMM)
#define N_COMM 0x12
#endif
#if !defined (N_FN)
#define N_FN 0x1f
#endif

#if !defined (N_EXT)
#define N_EXT 1
#endif
#if !defined (N_TYPE)
#define N_TYPE 036
#endif
#if !defined (N_STAB)
#define N_STAB 0340
#endif

/* The following type indicates the definition of a symbol as being
   an indirect reference to another symbol.  The other symbol
   appears as an undefined reference, immediately following this symbol.

   Indirection is asymmetrical.  The other symbol's value will be used
   to satisfy requests for the indirect symbol, but not vice versa.
   If the other symbol does not have a definition, libraries will
   be searched to find a definition.  */
#define N_INDR 0xa

/* The following symbols refer to set elements.
   All the N_SET[ATDB] symbols with the same name form one set.
   Space is allocated for the set in the text section, and each set
   element's value is stored into one word of the space.
   The first word of the space is the length of the set (number of elements).

   The address of the set is made into an N_SETV symbol
   whose name is the same as the name of the set.
   This symbol acts like a N_DATA global symbol
   in that it can satisfy undefined external references.  */

/* These appear as input to LD, in a .o file.  */
#define	N_SETA	0x14		/* Absolute set element symbol */
#define	N_SETT	0x16		/* Text set element symbol */
#define	N_SETD	0x18		/* Data set element symbol */
#define	N_SETB	0x1A		/* Bss set element symbol */

/* This is output from LD.  */
#define N_SETV	0x1C		/* Pointer to set vector in data area.  */

/* Warning symbol. The text gives a warning message, the next symbol
   in the table will be undefined. When the symbol is referenced, the
   message is printed.  */

#define	N_WARNING 0x1e

/* Weak symbols.  These are a GNU extension to the a.out format.  The
   semantics are those of ELF weak symbols.  Weak symbols are always
   externally visible.  The N_WEAK? values are squeezed into the
   available slots.  The value of a N_WEAKU symbol is 0.  The values
   of the other types are the definitions.  */
#define N_WEAKU	0x0d		/* Weak undefined symbol.  */
#define N_WEAKA 0x0e		/* Weak absolute symbol.  */
#define N_WEAKT 0x0f		/* Weak text symbol.  */
#define N_WEAKD 0x10		/* Weak data symbol.  */
#define N_WEAKB 0x11		/* Weak bss symbol.  */

#if !defined (N_RELOCATION_INFO_DECLARED)
/* This structure describes a single relocation to be performed.
   The text-relocation section of the file is a vector of these structures,
   all of which apply to the text section.
   Likewise, the data-relocation section applies to the data section.  */

struct relocation_info
{
#ifndef __MSHORT__
  /* Address (within segment) to be relocated.  */
  int r_address;
#else
  long int r_address;  /* Sigh, better than nothing.  */
#endif
  /* The meaning of r_symbolnum depends on r_extern.  */
  unsigned int r_symbolnum:24;
  /* Nonzero means value is a pc-relative offset
     and it should be relocated for changes in its own address
     as well as for changes in the symbol or section specified.  */
  unsigned int r_pcrel:1;
  /* Length (as exponent of 2) of the field to be relocated.
     Thus, a value of 2 indicates 1<<2 bytes.  */
  unsigned int r_length:2;
  /* 1 => relocate with value of symbol.
          r_symbolnum is the index of the symbol
	  in file's the symbol table.
     0 => relocate with the address of a segment.
          r_symbolnum is N_TEXT, N_DATA, N_BSS or N_ABS
	  (the N_EXT bit may be set also, but signifies nothing).  */
  unsigned int r_extern:1;
  /* Four bits that aren't used, but when writing an object file
     it is desirable to clear them.  */
#ifdef NS32K
  unsigned r_bsr:1;
  unsigned r_disp:1;
  unsigned r_pad:2;
#else
  unsigned int r_pad:4;
#endif
};
#endif /* no N_RELOCATION_INFO_DECLARED.  */

#endif /* __A_OUT_GNU_H__ */
