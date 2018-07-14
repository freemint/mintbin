/* flags.c -- main source file for flags.
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

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifndef ULONG_MAX
# define ULONG_MAX 0xffffffffUL
#endif

#include <getopt.h>
#include <error.h>

#include "targets.h"
#include "symbols.h"

#include "system.h"

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number (-V).  */
#define BE_SILENT  0
#define BE_NORMAL  1
#define BE_VERBOSE 2

static int verbosity = 1;       /* Be verbose (-v/-s).  */
static int do_print = 0;        /* Print flags.  */
static int plus_fastload = 0;
static int minus_fastload = 0;
static int plus_altload = 0;
static int minus_altload = 0;
static int plus_altalloc = 0;
static int minus_altalloc = 0;
static int plus_bestfit = 0;
static int minus_bestfit = 0;
static int plus_shtext = 0;
static int minus_shtext = 0;
static int mem_protected = 0;
static int mem_global = 0;
static int mem_super = 0;
static int mem_readable = 0;

static unsigned long plus_mask = 0;
static unsigned long minus_mask = 0;
static unsigned long protection_mask = 0;

/* When to print the names of files.  Not mutually exclusive in SYSV format.  */
static int filename_per_file = 0;	/* Once per file, on its own line.  */

static struct option long_options[] = {
  {"flags", required_argument, 0, 'f'},
  {"prg-flags", required_argument, 0, 'f'},
  {"print", no_argument, 0, 'P'},
  {"just-print", no_argument, 0, 'P'},
  {"help", no_argument, 0, 'h'},
  {"silent", no_argument, &verbosity, 0},
  {"quiet", no_argument, &verbosity, 0},
  {"verbose", no_argument, &verbosity, 2},
  {"version", no_argument, &show_version, 1},
  {"mfastload", no_argument, NULL, 'l'},
  {"mno-fastload", no_argument, NULL, 'l' + 256},
  {"mfastram", no_argument, NULL, 'r'},
  {"mno-fastram", no_argument, NULL, 'r' + 256},
  {"maltram", no_argument, NULL, 'r'},
  {"mno-altram", no_argument, NULL, 'r' + 256},
  {"mfastalloc", no_argument, NULL, 'a'},
  {"mno-fastalloc", no_argument, NULL, 'a' + 256},
  {"maltalloc", no_argument, NULL, 'a'},
  {"mno-altalloc", no_argument, NULL, 'a' + 256},
  {"mbest-fit", no_argument, NULL, 'B'},
  {"mno-best-fit", no_argument, NULL, 'B' + 256},
  {"mbaserel", no_argument, NULL, 'b'},
  {"mno-baserel", no_argument, NULL, 'b' + 256},
  {"mshared-text", no_argument, NULL, 'b'},
  {"mno-shared-text", no_argument, NULL, 'b' + 256},
  {"msharable-text", no_argument, NULL, 'b'},
  {"mno-sharable-text", no_argument, NULL, 'b' + 256},
  {"mprotected", no_argument, NULL, 'p'},
  {"mfull-protection", no_argument, NULL, 'p'},
  {"mmemory-protected", no_argument, &mem_protected, 'p'},
  {"mprivate-memory", no_argument, &mem_protected, 'p'},
  {"mglobal-memory", no_argument, &mem_global, 'g'},
  {"msuper-memory", no_argument, &mem_super, 'S'},
  {"mreadable-memory", no_argument, &mem_readable, 'R'},
  {"mreadonly-memory", no_argument, &mem_readable, 'R'},
  {0, no_argument, 0, 0}
};

static char* default_files[] =
{
  "a.out",
};

static void usage PARAMS ((FILE*, int));
static void help PARAMS ((void));

int
#if PROTOTYPES
main (int argc, char* argv[])
#else
     main (argc, argv)
     int argc;
     char* argv[];
#endif
{
  int the_option;
  char* tail;
  int has_flags = 0;
  int has_individual = 0;
  int fileind, lastind;
  int i;
  char** files;
  int status = EXIT_SUCCESS;
  unsigned long new_flags = 0;
  unsigned long old_flags = 0;

  if (argv != NULL)
    program_name = argv[0];

  if (program_name == NULL || program_name[0] == '\0')
    program_name = "flags";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "f:PhsqlaBbpgSsRrVv", 
				    long_options, (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, EXIT_SUCCESS);

	case 'f':
	  errno = 0;  /* Tsk, tsk, ...  */
          new_flags = strtoul (optarg, &tail, 0);
          if (new_flags == ULONG_MAX || optarg == tail)
	      error (EXIT_FAILURE, errno,
		     _("value `%s' no valid flag setting"), optarg);
	  has_flags = 1;
	  break;

        case 'l':
	  plus_fastload = 1;
	  minus_fastload = 0;
          break;

        case 'l' + 256:
	  plus_fastload = 0;
	  minus_fastload = 1;
          break;

        case 'r':
	  plus_altload = 1;
	  minus_altload = 0;
	  break;

        case 'r' + 256:
	  plus_altload = 0;
	  minus_altload = 1;
	  break;

	case 'a':
	  plus_altalloc = 1;
	  minus_altalloc = 0;
	  break;

	case 'a' + 256:
	  plus_altalloc = 0;
	  minus_altalloc = 1;
	  break;

	case 'B':
	  plus_bestfit = 1;
	  minus_bestfit = 0;
	  break;

	case 'B' + 256:
	  plus_bestfit = 0;
	  minus_bestfit = 1;
	  break;

	case 'b':
	  plus_shtext = 1;
	  minus_shtext = 0;
	  break;

	case 'b' + 256:
	  plus_shtext = 0;
	  minus_shtext = 1;
	  break;

	case 'p':
	  mem_protected = 1;
	  break;

	case 'g':
	  mem_global = 1;
	  break;

	case 'S':
	  mem_super = 1;
	  break;

	case 'R':
	  mem_readable = 1;
	  break;

	case 'P':
	  do_print = 1;
	  break;

        case 's':
        case 'q':
          verbosity = BE_SILENT;
          break;

	case 'V':
	  show_version = 1;
	  break;

        case 'v':
          verbosity = BE_VERBOSE;
          break;

	case 0:         /* A long option that just sets a flag.  */
	  break;

 	default:
	  help ();
	}
    }

  if (show_version)
    print_version (program_name);

  /* Check for illegal options combinations.  */
  if ((plus_fastload | minus_fastload
       | plus_altload | minus_altload
       | plus_altalloc | minus_altalloc
       | plus_bestfit | minus_bestfit
       | plus_shtext | minus_shtext
       | mem_protected | mem_global | mem_super
       | mem_readable) != 0)
    has_individual = 1;

  if (has_individual && has_flags)
    {
      error (EXIT_SUCCESS, 0,
	     _("can\'t set individual flags with option `-f'"));
      usage (stderr, EXIT_FAILURE);
    }

  /* Normalize protection options.  */
  if (mem_protected)
    mem_protected = 1;
  if (mem_global)
    mem_global = 1;
  if (mem_super)
    mem_super = 1;
  if (mem_readable)
    mem_readable = 1;

  if (mem_protected + mem_global + mem_super + mem_readable > 1)
    {
      error (EXIT_SUCCESS, 0, _("confliction memory protection flags"));
      usage (stderr, EXIT_FAILURE);
    }

  if (!BE_SILENT && plus_shtext)
    {
      error (EXIT_SUCCESS, 0, _("\
warning: changing the shared text flag is dangerous"));
      error (EXIT_SUCCESS, 0, _("\
(it normally requires recompilation)."));
    }
  if (!BE_SILENT && plus_bestfit)
    {
      error (EXIT_SUCCESS, 0, _("\
warning: changing the bestfit flag is dangerous"));
    }

  /* Calculate flags.  */
  if (has_flags)
    {
      plus_mask = new_flags;
      minus_mask = 0xffffffffUL;
    }
  else if (has_individual)
    {
      if (mem_protected)
        protection_mask = _MINT_F_MEMPRIVATE;
      else if (mem_global)
	protection_mask = _MINT_F_MEMGLOBAL;
      else if (mem_super)
	protection_mask = _MINT_F_MEMSUPER;
      else if (mem_readable)
	protection_mask = _MINT_F_MEMREADABLE;

      plus_mask = 0;
      if (plus_fastload)
	plus_mask |= _MINT_F_FASTLOAD;
      if (plus_altload) 
        plus_mask |= _MINT_F_ALTLOAD;
      if (plus_altalloc)
        plus_mask |= _MINT_F_ALTALLOC;
      if (plus_bestfit)
        plus_mask |= _MINT_F_BESTFIT;
      if (plus_shtext)
        plus_mask |= _MINT_F_SHTEXT;
      minus_mask = 0;
      if (minus_fastload)
	minus_mask |= _MINT_F_FASTLOAD;
      if (minus_altload) 
        minus_mask |= _MINT_F_ALTLOAD;
      if (minus_altalloc)
        minus_mask |= _MINT_F_ALTALLOC;
      if (minus_bestfit)
        minus_mask |= _MINT_F_BESTFIT;
      if (minus_shtext)
        minus_mask |= _MINT_F_SHTEXT;
    }
  else
    {
      do_print = 1;
      plus_mask = 0;
      minus_mask = 0;
    }

  if (optind == argc)
    {
      fileind = 0;
      files = default_files;
      lastind = 1;
    }
  else
    {
      fileind = optind;
      files = argv;
      lastind = argc;
    }

  if (fileind + 1 < lastind)
    filename_per_file = 1;

  for (i = fileind; i < lastind; i++)
    {
      struct mintbin_target* target = open_target (files[fileind],
						   (do_print ?
						   O_RDONLY : O_RDWR) | O_BINARY);
      unsigned long new_flags;
      char cflags[4];
      int had_error = 0;

      if (target == NULL)
	{
	  status = EXIT_FAILURE;
	  continue;
	}
      if (target->format != prg_target
	  && target->format != oldstyle_prg_target)
	{
	  status = EXIT_FAILURE;
	  error (EXIT_SUCCESS, 0, _("%s: not a MiNT executable"),
		 files[fileind]);
	  close_target (target);
	  continue;
	}

      new_flags = old_flags = target->execp.g_flags;
      new_flags &= ~minus_mask;
      new_flags |= plus_mask;
      if (mem_protected || mem_global || mem_super || mem_readable)
	{
	  new_flags &= ~_MINT_F_MEMFLAGS;
	  new_flags |= (protection_mask & _MINT_F_MEMFLAGS);
	}

      cflags[0] = (new_flags & 0xff000000) >> 24;
      cflags[1] = (new_flags & 0xff0000) >> 16;
      cflags[2] = (new_flags & 0xff00) >> 8;
      cflags[3] = new_flags & 0xff;

      if (!do_print)
	{
	  if (lseek (target->desc, 22, SEEK_SET) != 22)
	    {
	      error (EXIT_SUCCESS, errno, _("%s: seek error"),
		     files[fileind]);
	      had_error = 1;
	    }
	  if (!had_error && write (target->desc, cflags, 4) != 4)
	    {
	      error (EXIT_SUCCESS, errno,
		     _("%s: write error"), files[fileind]);
	      had_error = 1;
	    }
	}
      if (close_target (target) != 0)
	{
	  error (EXIT_SUCCESS, errno, _("%s: close error"),
		 files[fileind]);
	  had_error = 1;
	}

      if (!had_error && (do_print || verbosity > BE_SILENT))
	{
	  if (filename_per_file)
	    printf ("%s: ", files[fileind]);

	  if (verbosity >= BE_NORMAL)
	    {
	      printf (_("Current flags: 0x%08x"),
		      (unsigned) new_flags);
	      if (do_print == 0)
		printf (_(" (were 0x%08lx)"), old_flags);
	      printf (_(".\n"));
	    }

	  if (verbosity >= BE_VERBOSE)
	    {
	      if (new_flags & _MINT_F_FASTLOAD)
		printf (_("\
  Fastload flag is set (heap doesn\'t get cleared).\n"));
	      if (new_flags & _MINT_F_ALTLOAD)
		printf (_("\
  Altload flag is set (load into alternate RAM).\n"));
	      if (new_flags & _MINT_F_ALTALLOC)
		printf (_("\
  Altalloc flag is set (allocate memory from alternate RAM).\n"));
	      if (new_flags & _MINT_F_BESTFIT)
		printf (_("\
  Bestfit flag is set (space for stack reserved in BSS).\n"));
	      if (new_flags & _MINT_F_SHTEXT)
		printf (_("\
  Shared text flag is set (programs text segment is sharable).\n"));
	      switch (new_flags & _MINT_F_MEMPROTECTION)
		{
		default:
		  printf (_("\
  Program\'s memory is fully protected.\n"));
		  break;
		case _MINT_F_MEMGLOBAL:
		  printf (_("\
  Program\'s memory is not protected at all (globally accessible).\n"));
		  break;
		case _MINT_F_MEMSUPER:
		  printf (_("\
  Program\'s memory may be accessed in supervisor mode.\n"));
		  break;
		case _MINT_F_MEMREADABLE:
		  printf (_("\
  Program\'s memory is readable.\n"));
		  break;
		}
	    }

	  if (had_error)
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
  fputs (_("Display and manipulate program flags of MiNT executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -f, --flags=NEWFLAGS           set flags to hexadecimal, decimal or octal \n\
                                 value NEWFLAGS\n\
  -P, --print, --just-print      only print current flags\n\
  -l, --mfastload                set the fastload flag\n\
  -r, --mfastram, --maltram      set the fast ram (alternate or TT ram) flag\n\
  -a, --mfastalloc, --maltalloc  set the fast ram allocation flag\n\
  -B, --mbest-fit                set the best fit flag (stack in heap)\n\
  -b, --mbaserel, --mshared-text, --msharable-text\n\
                                 set the shared text flag\n\
  -p, --mprotected, --mfull-protection, --mmemory-protected\n\
      --mprivate-memory          turn full memory protection on\n\
  -g, --mglobal-memory           turn memory protection completely off\n\
  -S, --msuper-memory            allow memory access in supervisor mode\n\
  -R, --mreadable-memory, --mreadonly-memory\n\
                                 allow read access to memory\n\
  --mno-fastload                 unset the fastload flag\n\
  --mno-fastram, --mno-altram    unset the fast ram flags\n\
  --mno-fastalloc, --mno-altalloc\n\
                                 unset the fast ram allocation flag\n\
  --mno-best-fit                 unset the best fit flag (stack in heap)\n\
  --mno-baserel, --mno-shared-text, --mno-sharable-text\n\
                                 unset the shared text flag\n"),
	   stream);
  fputs (_("\
  -s, --silent                   print only error messages\n\
  -q, --quiet                    same as -s, --silent\n\
  -v, --verbose                  enable verbose diagnostic output\n"),
	 stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\n\
If INPUTFILE is missing, `a.out' is assumed.\n"),
	 stream);
  fputs (_("\
Arguments are cumulative.  An option that is contradictory to a previous\n\
one will override the previous setting.  Using the `--flags' resp. the `-f'\n\
or `--prg-flags' option is mutually exclusive with using one of the single\n\
flag arguments.  All options that change the memory protection flags are\n\
also mutually exclusive.  If you instruct the program to just print the \n\
current flags (option `--print' resp. `--just-print' or `-P'it is illegal\n\
specify any options that would change flags in order to avoid unintentional\n\
modifications.\n\
\n\
Flags that are not mentioned are left untouched in the executable.  The\n\
default action is to print out the current flags both in hexadecimal and\n\
decimal notation; in verbose all known flags, their current setting and\n\
a short description of their meaning are printed out.\n"), stream);
  fputs (_("Report bugs to <gufl0000@stud.uni-sb.de>\n"), stream);

  exit (status);
}

static void
help ()
{
  fprintf (stderr, _("Try `%s --help' for more information.\n"),
	   program_name);
  exit (EXIT_FAILURE);
}





