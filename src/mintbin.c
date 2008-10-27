/* mintbin.c -- main source file for mintbin.
   Copyright (C) 1998 Guido Flohr <gufl0000@stud.uni-sb.de>.

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

#if STDC_HEADERS
# include <stdio.h>
# include <stdlib.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#include <getopt.h>
#include <error.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#else
extern int errno;
#endif

#include "targets.h"

#include "system.h"

static void
usage PARAMS ((FILE*, int));

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number.  */

/* When to print the names of files.  Not mutually exclusive in SYSV format.  */
static int filename_per_file = 0;	/* Once per file, on its own line.  */

static struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

static int list_file PARAMS ((const char*));

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
    program_name = "mintbin";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "hV", long_options,
				    (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, 0);

	case 'V':
	  show_version = 1;
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
      return list_file ("a.out");
    } 
  else 
    {
      int i;
      for (i = optind; i < argc; i++) 
	{
	  if (list_file (argv[i]) != 0)
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
  fputs (_("Display information about given binary input files.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\n\
If INPUTFILE is missing, `a.out' is assumed.\n"),
	 stream);
  fputs (_("\n\
Report bugs to <gufl0000@stud.uni-sb.de>\n"), stream);

  exit (status);
}

static int
#if PROTOTYPES
list_file (const char* filename)
#else
list_file (filename)
     const char* filename;
#endif
{
  struct mintbin_target* target = open_target (filename, O_RDONLY);
  if (target == NULL)
    return -1;
  
  if (filename_per_file)
    fprintf (stdout, "%s: ", target->filename);

  switch (target->format) 
    {
    case aout_target:
      fputs (_("GNU style a.out object file.\n"), stdout);
      break;
    case ar_library_target:
      fputs (_("GNU style ar library.\n"), stdout);
      break;
    case oldstyle_ar_library_target:
      fputs (_("Old-style GNU ar library.\n"), stdout);
      break;
    case prg_target:
      if (target->execp.g_symbol_format == 0)
	{
	  switch (N_MAGIC (target->execp)) {
	  case OMAGIC:
	    fputs (_("MiNT impure executable.\n"), stdout);
	    break;
	  case NMAGIC:
	    fputs (_("MiNT pure executable.\n"), stdout);
	    break;
	  case ZMAGIC:
	    fputs (_("MiNT demand-paged executable.\n"), stdout);
	    break;
	  }
	}
      else
	{
	  switch (N_MAGIC (target->execp)) {
	  case OMAGIC:
	    fputs (_("MiNT impure executable with traditional symbol table.\n"),
		   stdout);
	    break;
	  case NMAGIC:
	    fputs (_("MiNT pure executable with traditional symbol table.\n"),
		   stdout);
	    break;
	  case ZMAGIC:
	    fputs (_("\
MiNT demand-paged executable with traditional symbol table.\n"),
		   stdout);
	    break;
	  }
	}
      break;
    case oldstyle_prg_target:
      fputs (_("Old-style GEMDOS executable.\n"), stdout);
      break;
    default:
      fputs (_("Unrecognized file format.\n"), stdout); 
      break;
    }

  if (close_target (target) != 0) 
    {
      error (EXIT_SUCCESS, errno, _("%s: warning: close error"),
	     target->filename);
    }

  return 0;
}










