/* csize.c -- main source file for csize.
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

#include <getopt.h>
#include <error.h>

#include "targets.h"
#include "symbols.h"

#include "system.h"

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number (-V).  */
static int sysv_format = 0;     /* Non-zero if SYSV format should be used.  */
static int radix = 10;          /* Output radix.  */
static const char* size_format;

static struct option long_options[] = {
  {"format", required_argument, 0, 'f' + 256},
  {"radix", required_argument, 0, 'r' + 256},
  {"target", required_argument, 0, 't' + 256},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

static char* default_files[] =
{
  "a.out",
};

static int header_printed = 0;

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
  int fileind, lastind;
  int i;
  char* tail;
  char** files;
  int status = EXIT_SUCCESS;

  if (argv != NULL)
    program_name = argv[0];

  if (program_name == NULL || program_name[0] == '\0')
    program_name = "csize";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "ABdoxhV", 
				    long_options, (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'f' + 256:
	  if (strcasecmp (optarg, "sysv") == 0)
	    sysv_format = 1;
	  else if (strcasecmp (optarg, "bsd") == 0)
	    sysv_format = 0;
	  else if (strcasecmp (optarg, "berkeley") == 0)
	    sysv_format = 0;
	  else
	    {
	      error (EXIT_SUCCESS, 0, _("\
invalid argument to `--format'"));
	      usage (stderr, EXIT_FAILURE);
	    }
	  break;

	case 'A':
	  sysv_format = 1;
	  break;

	case 'B':
	  sysv_format = 0;
	  break;

	case 'd':
	  radix = 10;
	  break;

	case 'o':
	  radix = 8;
	  break;

	case 'x':
	  radix = 16;
	  break;

	case 'r' + 256:
	  radix = strtol (optarg, &tail, 0);
	  if (radix != 8 && radix != 10 && radix != 16)
	    {
	      error (EXIT_SUCCESS, 0, _("invalid radix"));
	      usage (stderr, EXIT_FAILURE);
	    } 
	  break;

	case 'h':
	  usage (stdout, EXIT_SUCCESS);

	case 'V':
	  show_version = 1;
	  break;

	case 0:         /* A long option that just sets a flag.  */
	  break;

 	default:
	  help ();
	}
    }

  if (show_version)
    print_version (program_name);

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

  if (radix == 8)
    {
      if (sysv_format)
	size_format = "%#10lo";
      else
	size_format = "%#9lo";
    }
  else if (radix == 10)
    {
      if (sysv_format)
	size_format = "%10lu";
      else
	size_format = "%9lu";
    }
  else
    {
      if (sysv_format)
	size_format = "%#10lx";
      else
	size_format = "%#9lx";
    }
  for (i = fileind; i < lastind; i++)
    {
      struct mintbin_target* target = open_target (files[i],
						   O_RDONLY);
      if (target == NULL)
	{
	  status = EXIT_FAILURE;
	  continue;
	}
      if (target->format != oldstyle_prg_target
	  && target->format != prg_target)
	{
	  status = EXIT_FAILURE;
	  error (EXIT_SUCCESS, 0, _("%s: not a MiNT executable file"),
		 files[i]);
	  error (EXIT_SUCCESS, 0, _("(use `size' instead)"));
	  close_target (target);
	  continue;
	}

      if (!header_printed)
	{
	  if (!sysv_format)
	    printf ("%9s %9s %9s %9s %9s %s\n",
		    "text",
		    "data",
		    "bss",
		    radix == 8 ? "oct" : "dec",
		    "hex",
		    _("filename"));
	  header_printed = 1;
	}

      if (sysv_format)
	{
	  size_t address = 0;

	  /* Header.  */
	  printf ("%s:\n", files[i]);
	  printf (_("section     size       addr\n"));

	  /* Text segment.  */
	  printf (".text ");
	  printf (size_format, target->execp.g_text);
	  putc (' ', stdout);
	  printf (size_format, address);
	  putc ('\n', stdout);
	  address += target->execp.g_text;

	  /* Data segment.  */
	  printf (".data ");
	  printf (size_format, target->execp.a_data);
	  putc (' ', stdout);
	  printf (size_format, address);
	  putc ('\n', stdout);
	  address += target->execp.a_data;

	  /* Bss segment.  */
	  printf (".bss  ");
	  printf (size_format, target->execp.g_text);
	  putc (' ', stdout);
	  printf (size_format, address);
          putc ('\n', stdout);
	  address += target->execp.a_bss;

	  /* Sum.  */
	  printf (_("Total "));
	  printf (size_format, address);
	  printf ("\n\n\n");
  	}
      else
	{
	  printf (size_format, target->execp.g_text);
	  putc (' ', stdout);
	  printf (size_format, target->execp.a_data);
	  putc (' ', stdout);
	  printf (size_format, target->execp.a_bss);
	  if (radix == 8)
	    printf (" %9lo", target->execp.g_text + target->execp.a_data
		    + target->execp.a_bss);
	  else
	    printf (" %9lu", target->execp.g_text + target->execp.a_data
		    + target->execp.a_bss);
	  printf (" %9lx", target->execp.g_text + target->execp.a_data
		  + target->execp.a_bss);
	  printf (" %s\n", files[i]);
	}

      if (close_target (target) != 0)
	{
	  error (EXIT_SUCCESS, errno, _("%s: close error"),
		 files[fileind]);
	}

    }
  
  return EXIT_SUCCESS;
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
  fputs (_("Display section sizes of old-style GEMDOS executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -A                             output in SysV format\n\
  -B                             output in Berkeley (BSD) format\n\
  --format=FORMAT                output in FORMAT\n\
  -d                             output in decimal format (default)\n\
  -o                             output in octal format\n\
  -x                             output in hexadecimal format\n\
  --radix=RADIX                  output with RADIX as numerical radix\n\
  --target=TARGET                input files are in format TARGET\n"),
	   stream);

  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\n\
If INPUTFILE is missing, `a.out' is assumed.\n"),
	 stream);
  fputs (_("\
The default output format is `format=berkeley'.  The output format\n\
`bsd' is a synonyme for `berkeley', the other choice is `sysv'.  Case is\n\
always irrelevant.\n\
The numerical radix must be one of 10 (decimal, default), 8 (octal) or\n\
16 (hexadecimal).\n\
Note that the output for executables in the extended MiNT format can\n\
differ from the size reported by the `size' program because `size'\n\
always reports the real section sizes whereas this program reports the\n\
kernel's notion of the segment sizes.\n"), stream);

  fprintf (stream, _("\
%s: supported target: a.out-mintprg gemdos\n"),
		     program_name);

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

