/* symex.c -- main source file for symex.
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

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif

#include <getopt.h>
#include <error.h>

#include <a.out.h>

#include <libmb.h>

#include "targets.h"
#include "symbols.h"

#include "system.h"

static void
usage PARAMS ((FILE*, int));

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number (-V).  */

static struct option long_options[] = {
  {"output", required_argument, 0, 'o'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

char* killfile = NULL;

static int extract_symbols PARAMS ((const char*, const char*));
static void kill_file PARAMS ((void));
static RETSIGTYPE cleanup PARAMS ((int));

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
  char* output_filename = NULL;

  if (argv != NULL)
    program_name = argv[0];

  if (program_name == NULL || program_name[0] == '\0')
    program_name = "symex";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "hVo:", long_options,
				    (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, EXIT_SUCCESS);

        case 'o':
          output_filename = optarg;
          break;

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

  (void) signal (SIGTERM, cleanup);
  (void) signal (SIGINT, cleanup);
#ifdef SIGQUIT
  (void) signal (SIGQUIT, cleanup);
#endif
#ifdef SIGHUP
  (void) signal (SIGHUP, cleanup);
#endif

  if (optind == argc) 
    {
      /* No input files.  Try it with a.out.  */
      status = extract_symbols ("a.out", output_filename);
    }
  else
    {
      status = extract_symbols (argv[optind], output_filename);
    }

  killfile = NULL;
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
  fputs (_("Extract symbol table from MiNT executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -o, --output=FILENAME          set output FILENAME\n"),
	 stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\
If INPUTFILE is missing, ``a.out'' is assumed.  If output filename\n\
is missing output is written to standard output.\n"),
	 stream);
  fputs (_("\
Report bugs to <gufl0000@stud.uni-sb.de>\n"), stream);

  exit (status);
}

static int
#if PROTOTYPES
extract_symbols (const char* filename, const char* output_filename)
#else
extract_symbols (filename, output_filename)
     const char* filename;
     const char* output_filename;
#endif
{
  struct mintbin_target* target = open_target (filename, O_RDONLY);
  int status = 0;
  int output_desc;
  unsigned char* buffer = NULL;

  if (target == NULL)
    return -1;
  
  if (target->format != prg_target)
    {
      error (EXIT_SUCCESS, 0, _("%s: file format not recognized"),
	     target->filename);
      return -1;
    }
  
  if (target->execp.g_symbol_format != _GNU_FORMAT)
    {
      error (EXIT_SUCCESS, 0, _("%s: file contains no valid symbol table"),
	     target->filename);
      return -1;
    }
  else if (target->execp.a_syms == 0)
    {
      error (EXIT_SUCCESS, 0, _("%s: file contains no symbols"),
	     target->filename);
      return -1;
    }

  if (output_filename == NULL)
    {
      output_filename = _("standard output");
      output_desc = fileno (stdout);
    }
  else
    {
      killfile = (char*) output_filename;
      (void) atexit (kill_file);
      output_desc = open (output_filename, O_WRONLY | O_CREAT | O_TRUNC,
			  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
			  | S_IROTH | S_IWOTH);
      if (output_desc < 0)
	{
          error (EXIT_FAILURE, errno, _("%s: open error"),
		 output_filename);
	}
    }

  /* Write out header.  */
  buffer = xmalloc (1024);
  memset (buffer, 0, 1024);
  /* Write ZMAGIC.  */
  buffer[2] = 0x01;
  buffer[3] = 0x0b;
  buffer[16] = (target->execp.a_syms & 0xff000000) >> 24;
  buffer[17] = (target->execp.a_syms & 0xff0000) >> 16;
  buffer[18] = (target->execp.a_syms & 0xff00) >> 8;
  buffer[19] = (target->execp.a_syms & 0xff);

  if (write (output_desc, (const void*) buffer, 1024) != 1024)
    {
      error (EXIT_FAILURE, errno, _("%s: write error"),
	     output_filename);
    }

  free (buffer);
  buffer = NULL;

  if (lseek (target->desc, N_SYMOFF (target->execp), SEEK_SET) == -1)
    {
      error (EXIT_FAILURE, errno, _("%s: seek error"),
	     target->filename);
    }

  /* Slurp in the symbols plus the strings.  */
  buffer = xmalloc (target->execp.g_syms);
  if (read (target->desc, buffer, target->execp.g_syms)
      != target->execp.g_syms)
    {
      error (EXIT_FAILURE, errno, _("%s: read error"),
	     target->filename);
    }
  
  if (write (output_desc, (const void*) buffer, target->execp.g_syms)
      != target->execp.g_syms)
    {
      error (EXIT_FAILURE, errno, _("%s: write error"),
	     output_filename);
    }

  free (buffer);

  if (close_target (target) != 0) 
    {
      error (EXIT_SUCCESS, errno, _("%s: warning: close error"),
	     target->filename);
      status = -1;
    }
  
  return status;
}

static void
#if PROTOTYPES
kill_file (void)
#else
kill_file ()
#endif
{
  if (killfile != NULL)
    (void) unlink (killfile);
  killfile = NULL;
}

static RETSIGTYPE
#if PROTOTYPES
cleanup (int signum)
#else
cleanup (signum)
     int signum;
#endif
{
  kill_file ();
}



