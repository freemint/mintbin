/* cnm.c -- main source file for cnm.
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
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#else
extern int errno;
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#include <getopt.h>
#include <error.h>

#include "targets.h"
#include "symbols.h"

#include "system.h"

static void
usage PARAMS ((FILE*, int));

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number (-V).  */
static int globals_only = 0;    /* Display only global symbols.  */
static int unsorted = 0;        /* Do not sort symbols.  */

/* When to print the names of files.  */
static int filename_per_file = 0;	/* Once per file, on its own line.  */

static struct option long_options[] = {
  {"global-only", no_argument, &globals_only, 1},
  {"extern-only", no_argument, &globals_only, 1},
  {"unsorted", no_argument, &unsorted, 1},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

static int print_symbols PARAMS ((const char*));
static int cmp_symbols PARAMS ((const void* sym1, const void* sym2));

int
#if PROTOTYPES
main (int argc, char* argv[])
#else
     main (argc, argv)
     int argc;
     char* argv[];
#endif
{
  int status = EXIT_SUCCESS;
  int the_option;

  if (argv != NULL)
    program_name = argv[0];

  if (program_name == NULL || program_name[0] == '\0')
    program_name = "cnm";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "hgVu", long_options,
				    (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, EXIT_SUCCESS);

	case 'V':
	  show_version = 1;
	  break;

	case 'g':
	  globals_only = 1;
	  break;

	case 'u':
	  unsorted = 1;
	  break;

	case 0:         /* A long option that just sets a flag.  */
	  break;

 	default:
	  usage (stderr, 1);
	}
    }

  if (show_version)
    print_version (program_name);

  if (optind + 1 < argc)
    filename_per_file = 1;

  if (optind == argc) 
    {
      /* No input files.  Try it with a.out.  */
      return print_symbols ("a.out");
    }
  else 
    {
      int i;

      for (i = optind; i < argc; i++) 
	{
	  if (print_symbols (argv[i]) != 0)
	    status = EXIT_FAILURE;
	}
    }

  return status;
}

static void
#if PROTOTYPES
usage (FILE* stream, int status)
#else
     usage (stream, status)
     FILE* stream;
     int status;
#endif
{
  fprintf (stream, _("Usage: %s [OPTIONS] INPUTFILE ...\n"), program_name);
  fputs (_("List symbols of old-style GEMDOS executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -g, --globals-only, --extern-only\n\
                                 display only global symbols\n\
  -u, --unsorted                 do not sort symbols by value\n"),
	   stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\n\
If INPUTFILE is missing, `a.out' is assumed.\n"),
	 stream);
  fputs (_("\n\
The output format is non-standard (but traditionally used with this\n\
program).\n"), stream);

  fputs (_("Report bugs to <gufl0000@stud.uni-sb.de>.\n"), stream);

  exit (status);
}

static int
#if PROTOTYPES
print_symbols (const char* filename)
#else
print_symbols (filename)
     const char* filename;
#endif
{
  struct mintbin_target* target = open_target (filename, O_RDONLY | O_BINARY);
  int status = 0;

  if (target == NULL)
    return -1;
  
  if (!(target->format == oldstyle_prg_target ||
	(target->format == prg_target
	 && target->execp.g_symbol_format == _DRI_FORMAT)))
    {
      error (EXIT_SUCCESS, 0, _("\
%s: file format not recognized, use `nm' instead"),
	     target->filename);
      status = -1;
    }
  
  if (status == 0)
    {
      /* Setting the PRESERVE argument of SWAP_SYMTAB_IN to non-zero 
	 causes the function to store the original type in the n_desc
	 member of struct nlist.  */
      if (swap_symtab_in (target, 1) != 0 || target->symtab == NULL)
	status = -1;
    }

  if (status == 0)
    {
      size_t i;
      size_t slots = target->execp.g_syms / 14;

      if (filename_per_file)
	printf ("\n%s:\n", target->filename);
      
      if (slots == 0)
	printf (_("no slots mem %lu\n"), target->execp.g_syms);
      else if (slots == 1)
	printf (_("one slot mem %lu\n"), target->execp.g_syms);
      else if (slots % 10 < 2 || slots % 10 > 4)
	/* Note for translators: Use this string for numbers whose
	   last digit is not 2, 3, or 4.  */
	printf (_("%lu slots mem %lu\n"), slots, target->execp.g_syms);
      else 
	/* Note for translators: Use this string for numbers whose
	   last digit is 2, 3, or 4.  */
	printf (_("%lu slots mem %lu \n"), slots, target->execp.g_syms);

      if (globals_only)
	{
	  /* Sigh, we have to scan through the entire table first to
	     find out the number of global symbols.  */
	  size_t global_symbols = 0;
	  size_t i;
	  for (i = 0; i < target->symcount; i++)
	    {
	      if (target->symtab[i].n_desc & A_GLOBL
		  || (target->symtab[i].n_desc & A_DEF
		      && target->symtab[i].n_desc & A_EXT))
		global_symbols++;
	    }
	  if (global_symbols == 0)
	    printf (_("no global symbols\n"));
	  else if (global_symbols == 1)
	    printf (_("one global symbol\n"));
	  else if (global_symbols % 10 < 2 || global_symbols % 10 > 4)
	    /* Note for translators: Use this string for numbers whose
	       last digit is not 2, 3, or 4.  */
	    printf (_("%lu global symbols\n"), global_symbols);
	  else 
	    /* Note for translators: Use this string for numbers whose
	       last digit is 2, 3, or 4.  */
	    printf (_("%lu global symbols \n"), global_symbols);
	}
      else
	{
	  size_t symbols = target->symcount;

	  if (symbols == 0)
	    printf (_("no symbols\n"));
	  else if (symbols == 1)
	    printf (_("one symbol\n"));
	  else if (symbols % 10 == 2 || symbols % 10 == 3 || symbols % 10 == 4)
	    /* Note for translators: Use this string for numbers whose
	       last digit is 2, 3, or 4.  */
	    printf (_("%lu symbols\n"), target->symcount);
	  else 
	    /* Note for translators: Use this string for numbers whose
	       last digit is not 2, 3, or 4.  */
	    printf (_("%lu symbols \n"), target->symcount);
	}

      if (!unsorted)
	qsort (target->symtab, target->symcount, sizeof (struct nlist),
	       cmp_symbols);

      for (i = 0; i < target->symcount; i++)
	{
	  char* name = target->symtab[i].n_un.n_name;

	  if (globals_only
	      && !(target->symtab[i].n_desc & A_GLOBL
		   || (target->symtab[i].n_desc & A_DEF
		       && target->symtab[i].n_desc & A_EXT)))
	    continue;

	  printf ("%-22.22s ", name);
	  printf ("%8lx ", target->symtab[i].n_value);

	  if (target->symtab[i].n_desc == A_TFILE)
	    printf (_("text file "));
	  /* No A_TFARC, clashes with A_LNAM and thus impossible.  */
	  else
	    {
	      int is_global = (target->symtab[i].n_desc & A_GLOBL
			       || (target->symtab[i].n_desc & A_DEF
				   && target->symtab[i].n_desc & A_EXT));

	      if (target->symtab[i].n_desc & A_EXT)
		printf (_("external "));
	      if (target->symtab[i].n_desc & A_EQREG)
		printf (_("equated register "));
	      if (target->symtab[i].n_desc & A_GLOBL)
		printf (_("global "));
	      if (target->symtab[i].n_desc & A_EQU)
		printf (_("equated "));
	      if (target->symtab[i].n_desc & A_DEF)
		printf (_("defined "));
	      if (target->symtab[i].n_desc & A_BSS)
		{
		  if (is_global)
		    printf (_("Bss "));
		  else
		    printf (_("bss "));
		}
	      if (target->symtab[i].n_desc & A_TEXT)
		{
		  if (is_global)
		    printf (_("Text "));
		  else
		    printf (_("text "));
		}
	      if (target->symtab[i].n_desc & A_DATA)
		{
		  if (is_global)
		    printf (_("Data "));
		  else
		    printf (_("data "));
		}
	    }
	  printf ("(%#06x)\n", (unsigned) target->symtab[i].n_desc);
	}
    }

  if (close_target (target) != 0) 
    {
      error (EXIT_SUCCESS, errno, _("%s: warning: close error"),
	     target->filename);
      status = -1;
    }
  
  return status;
}

static int
#if PROTOTYPES
cmp_symbols (const void* sym1, const void* sym2)
#else
cmp_symbols (sym1, sym2)
     const void* sym1;
     const void* sym2;
#endif
{
  long diff = (((struct nlist*) sym1)->n_value
	       - ((struct nlist*) sym2)->n_value);
  return (int) ((diff > 0) - (diff < 0));
}

