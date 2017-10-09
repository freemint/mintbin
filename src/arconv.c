/* arconv.c -- main source file for arconv.
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

#include "ar.h"

#include "targets.h"
#include "symbols.h"

#include "system.h"

static void
usage PARAMS ((FILE*, int));

char* program_name = NULL;

/* Command options.  */
static int show_version = 0;    /* Show the version number (-V).  */
static int verbose = 0;         /* Be verbose (-v).  */

static struct option long_options[] = {
  {"output", required_argument, 0, 'o'},
  {"verbose", no_argument, &verbose, 1},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

char* killfile = NULL;

static int arconv PARAMS ((const char*, const char*));
static int convert_module PARAMS ((const char*, int,
				   const char*, int));
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
    program_name = "arconv";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "hvVo:", long_options,
				    (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, EXIT_SUCCESS);

        case 'v':
          verbose = 1;
          break;

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
      /* No input files.  */
      error (EXIT_FAILURE, 0, _("no input files"));
    }
  else if (output_filename == NULL)
    {
      error (EXIT_FAILURE, 0, _("no output file"));
    }
  else
    {
      status = arconv (argv[optind], output_filename);
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
  fputs (_("Convert old-style libraries to new format.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -o, --output=FILENAME          set output FILENAME\n\
  -v, --verbose                  list modules while processing\n"),
	 stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);
  fputs (_("\n\
Report bugs to <gufl0000@stud.uni-sb.de>\n"), stream);

  exit (status);
}

static int
#if PROTOTYPES
arconv (const char* filename, const char* output_filename)
#else
arconv (filename, output_filename)
     const char* filename;
     const char* output_filename;
#endif
{
  struct mintbin_target* target = open_target (filename, O_RDONLY);
  int status = 0;
  int output_desc;
  
  if (target == NULL)
    return -1;
  
  if (target->format != oldstyle_ar_library_target)
    {
      error (EXIT_SUCCESS, 0, _("%s: not an old-style ar archive"),
	     target->filename);
      return -1;
    }
  
  /* We have to lseek back to first module.  */  
  if (lseek (target->desc, OLD_SARMAG, SEEK_SET) == -1)
    {
      error (EXIT_FAILURE, errno, _("%s: seek error"),
	     target->filename);
    }

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

  /* Write out header.  */
  if (write (output_desc, ARMAG, SARMAG) != SARMAG)
    {
      error (EXIT_FAILURE, errno, _("%s: write error"),
	     output_filename);
    }


  while (convert_module (filename, target->desc,
			 output_filename, output_desc) == 0);

  if (close (output_desc) != 0)
    {
      error (EXIT_FAILURE, errno, _("%s: close error"),
	     output_filename);
    }

  if (close_target (target) != 0) 
    {
      error (EXIT_SUCCESS, errno, _("%s: warning: close error"),
	     target->filename);
      status = -1;
    }
  
  if (status == 0 && verbose)
    printf ("Do not forget to run ranlib on ``%s''!\n",
	    output_filename);

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

static int
#if PROTOTYPES
convert_module (const char* input_filename, int input_desc,
		const char* output_filename, int output_desc)
#else
convert_module (input_filename, intput_desc,
		output_filename, output_desc)
#endif
{
#define OLD_HEADER_SIZE 58
#define NEW_HEADER_SIZE 60

   struct oldstyle_ar_hdr oldhdr;
   struct ar_hdr newhdr;
   int read_bytes;
   unsigned long module_size;
   unsigned long count;
   char buf[1024];
   char* tail = NULL;
   int is_symdef = 0;

   read_bytes = read (input_desc, &oldhdr, OLD_HEADER_SIZE);

   if (read_bytes == 0)
     {
       return EOF;
     }
   else if (read_bytes != OLD_HEADER_SIZE)
     {
       error (EXIT_FAILURE, errno, _("%s: read error"),
	      input_filename);
     }

   /* Now copy the members of the module headers.  The ordering may
      seem strange but it this make sure that the strings are 
      null-terminated while they may be printed.  */
   memset (&newhdr, ' ', NEW_HEADER_SIZE);
   memcpy (newhdr.ar_name, oldhdr.ar_name, 16);
   if (strncmp ("__.SYMDEF", newhdr.ar_name, 9) == 0)
     is_symdef = 1;

   if (verbose) {
     newhdr.ar_name[16] = '\0';
     if (is_symdef)
       puts ("skipping module __.SYMDEF");
     else
       printf ("copying module %s\n", newhdr.ar_name);
   }

   memcpy (newhdr.ar_size, oldhdr.ar_size, 10);
   module_size = strtoul (newhdr.ar_size, &tail, 10);
   if (tail == newhdr.ar_size)
     {
       newhdr.ar_size[10] = '\0';
       error (EXIT_FAILURE, 0, _("\
invalid module size ``%s'' for module %s"),
	      newhdr.ar_size, newhdr.ar_name);
     }

   /* Modules begin on byte boundaries.  */
   module_size += (module_size % 2);

   memcpy (newhdr.ar_date, oldhdr.ar_date, 12);
   memcpy (newhdr.ar_uid, oldhdr.ar_uid, 4);
   memcpy (newhdr.ar_gid, oldhdr.ar_gid, 4);
   memcpy (newhdr.ar_mode, oldhdr.ar_mode, 8);
   memcpy (newhdr.ar_fmag, ARFMAG, 2);

   /* Write out new module header.  */
   if (!is_symdef
       && write (output_desc, &newhdr, NEW_HEADER_SIZE) != NEW_HEADER_SIZE)
     {
       error (EXIT_FAILURE, errno, _("%s: write error"),
	      output_filename);
     }

   for (count = 0; count < module_size; count += 1024)
     {
       int chunk_size = 1024;
       
       if (count + chunk_size > module_size)
	 chunk_size = module_size - count;

       read_bytes = read (input_desc, buf, chunk_size);
       if (read_bytes < 0)
	 error (EXIT_FAILURE, errno, _("%s: read error"),
		input_filename);
       else if (read_bytes != chunk_size)
	 error (EXIT_FAILURE, 0, _("%s: file truncated"),
		input_filename);
       if (!is_symdef && write (output_desc, buf, chunk_size) != chunk_size)
	 error (EXIT_FAILURE, errno, _("%s: write error"),
		output_filename);
     }

   return 0;
}
