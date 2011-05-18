/* stack.c -- main source file for stack.
   Copyright (C) 1998, 1999 Guido Flohr <gufl0000@stud.uni-sb.de>.

This file is part of MiNTbin.

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
static int fix_stack = 0;       /* Change the stack size (-S).  */
#define BE_SILENT  0
#define BE_NORMAL  1
#define BE_VERBOSE 2
static int verbosity = 1;       /* Be verbose (-v/-s).  */
static long newstack = 0;       /* New stack size (normalized).  */

/* When to print the names of files.  Not mutually exclusive in SYSV format.  */
static int filename_per_file = 0;	/* Once per file, on its own line.  */

static struct option long_options[] = {
  {"fix", required_argument, 0, 'S'},
  {"size", required_argument, 0, 'S'},
  {"print", no_argument, 0, 'P'},
  {"help", no_argument, 0, 'h'},
  {"silent", no_argument, &verbosity, 0},
  {"quiet", no_argument, &verbosity, 0},
  {"verbose", no_argument, &verbosity, 2},
  {"version", no_argument, &show_version, 1},
  {0, no_argument, 0, 0}
};

static int print_stack PARAMS ((const char*));
static int extract_size PARAMS ((long*, const char*));
static int get_stackpos PARAMS ((struct mintbin_target*));

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
    program_name = "stack";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((the_option = getopt_long (argc, argv, "hS:PsqVv", long_options,
				    (int *) 0)) != EOF)
    {
      switch (the_option)
	{
	case 'h':
	  usage (stdout, EXIT_SUCCESS);

        case 'S':
          fix_stack = 1;
          if (extract_size (&newstack, optarg) != 0)
	    usage (stderr, EXIT_FAILURE);
          break;

	case 'P':
	  fix_stack = 0;
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
	  usage (stderr, 1);
	}
    }

  if (show_version)
    print_version (program_name);

  if (verbosity < BE_NORMAL && !fix_stack)
    {
      error (EXIT_FAILURE, 0, _("\
Nothing to do if silent and not fixing stack size"));
    }

  if (optind + 1 < argc)
    filename_per_file = 1;

  if (optind == argc) 
    {
      /* No input files.  Try it with a.out.  */
      return print_stack ("a.out");
    }
  else 
    {
      int i;
      for (i = optind; i < argc; i++) 
	{
	  if (print_stack (argv[i]) != 0)
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
  fputs (_("Display and manipulate stack setting of MiNT executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -P, --print                    just print stack size\n\
  -S, --fix=SIZE, --size=SIZE    change stack size to SIZE bytes\n"),
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
  fputs (_("\n\
The stacksize is only modified when option -S is given.  The SIZE\n\
argument may optionally be followed by one of the modifiers `k'\n\
(kilo bytes) or `M' (mega bytes).\n"), stream);
  fputs (_("Report bugs to <gufl0000@stud.uni-sb.de>.\n"), stream);

  exit (status);
}

static int
#if PROTOTYPES
print_stack (const char* filename)
#else
print_stack (filename)
     const char* filename;
#endif
{
  struct mintbin_target* target = open_target (filename, fix_stack
					       ? O_RDWR : O_RDONLY);
  int status = 0;

  if (target == NULL)
    return -1;
  
  if (target->format != prg_target
      && target->format != oldstyle_prg_target
      && target->format != aout_target) {
    error (EXIT_SUCCESS, 0, _("%s: file format not recognized"),
	   target->filename);
    return -1;
  }

  if (get_stackpos (target) < 0)
    status = -1;  /* The function has already reported the error.  */

  if (status == 0 && target->execp.g_stkpos < N_TXTOFF (target->execp))
    {
      if (target->execp.g_stkpos == 0)
	error (EXIT_SUCCESS, 0, _("\
%s: no stack information available."),
	       target->filename);
      else
	error (EXIT_SUCCESS, 0, _("\
%s: invalid file position (%lu) for stack size"),
	       target->filename, target->execp.g_stkpos);
      status = -1;
    }
    
  if (status == 0 && verbosity > BE_SILENT)
    {
      if (lseek (target->desc, target->execp.g_stkpos, SEEK_SET)
	  != target->execp.g_stkpos)
	{
	  error (EXIT_SUCCESS, errno, _("%s: seek error"),
		 target->filename);
	  status = -1;
	}
      else
	{
	  unsigned char buf[4];

	  errno = 0;
	  if (read (target->desc, (void*) buf, 4) != 4)
	    {
	      if (errno != 0)
		error (EXIT_SUCCESS, errno, _("%s: read error"),
		       target->filename);
	      else
		error (EXIT_SUCCESS, 0, _("%s: file truncated"),
		       target->filename);
	    }
	  else
	    {
	      long stksize = get32 (buf);

	      if (filename_per_file)
		printf (_("%s: stack size: %ld bytes (%ld kilo bytes).\n"),
			target->filename, stksize, stksize >> 10);
	      else
		printf (_("stack size: %ld bytes (%ld kilo bytes).\n"),
			stksize, stksize >> 10);

	      if (verbosity >= BE_VERBOSE)
		{
		  switch (stksize)
		    {
		    case -1L:
		      printf (_("\
(This means really that almost all of the available memory will\n\
be used for the stack.  If you run the image you will not be\n\
able to run any other process until the current one finishes.\n"));
		      break;
		    case 0L:
		      printf (_("\
(This means really that only a minimum amount of memory will\n\
be reserved for the stack.  This minimum will usually be 8192\n\
bytes and is sufficient for most purposes.  However, if your\n\
program is ``stack-intensive'' such as programs with support\n\
for regular expressions it is probably not enough.\n"));
		      break;
		    case 1L:
		      printf (_("\
(This means really that one quarter of the available memory will\n\
be reserved for the stack.  This may be necessary for very\n\
``stack-intensive'' applications such as compilers.  In most cases\n\
however it is simply too much.\n"));
		      break;
		    case 2L:
		      printf (_("\
(This means really that one half of the available memory will\n\
be reserved for the stack.  This may be necessary for extremely\n\
``stack-intensive'' applications such as compilers.  In most cases\n\
however it is simply too much.\n"));
		      break;
		    case 3L:
		      printf (_("\
(This means really that three quarters of the available memory will\n\
be reserved for the stack.  This may be necessary for extremely\n\
``stack-intensive'' applications such as compilers.  In most cases\n\
however this is overkill and waste of RAM.\n"));
		      break;
		    default:
		      if (stksize < 0)
			printf (_("\
(This means really that %ld bytes will be reserved for the stack.\n\
The negative value signifies that the heap will be used for dynamic\n\
memory allocation."), -stksize);
		      else
			/* Note for translators:  The open parantheses
			   will be continued on the line talking about
			   the bus error resulting from invalid stack
			   sizes.  */
			printf (_("("));
		      break;
		    }
		  printf (_("\
An invalid resp. insufficient setting for the program stack size\n\
may result in a bus error, sometimes only in special cases.)\n"));
		}
	    }
	}
    }

  if (status == 0 && fix_stack)
    {
      if (lseek (target->desc, target->execp.g_stkpos, SEEK_SET) !=
	  target->execp.g_stkpos)
	{
	  error (EXIT_SUCCESS, errno, _("%s: seek error"),
		 target->filename);
	  status = -1;
	}
      else
	{
	  unsigned char buf[4];

	  buf[0] = (newstack & 0xff000000) >> 24;
	  buf[1] = (newstack & 0xff0000) >> 16;
	  buf[2] = (newstack & 0xff00) >> 8;
	  buf[3] = (newstack & 0xff);
	  if (write (target->desc, (void*) buf, 4) != 4)    
	    {
	      error (EXIT_SUCCESS, 0, _("%s: write error"),
		     target->filename);
	      status = -1;
	    }
	  if (verbosity > BE_SILENT)
	    printf (_("Fixed stack to %ld bytes (%ld kilo bytes).\n"),
		    newstack, newstack >> 10);
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
extract_size (long* size, const char* optarg)
#else
extract_size (size, optarg)
     long* size;
     const char* optarg;
#endif
{
  char* tail;

  errno = 0;
  *size = strtol (optarg, &tail, 0);
  if (tail == optarg || errno != 0)
    {
      error (EXIT_SUCCESS, errno, _("not a valid stack size: ``%s''"),
	     optarg);
      return -1;
    }

  if ((tail[0] == 'k' || tail[0] == 'K') && tail[1] == '\0')
    {
      if (*size <= -0x200000 || *size > 0x1fffff)
	{
	  error (EXIT_SUCCESS, 0, _("\
stack size ``%s'' out of range: overflow in number"), optarg);
	  return -1;
	}
      *size <<= 10;
    }

  if ((tail[0] == 'm' || tail[0] == 'M') && tail[1] == '\0')
    {
      if (*size < -0x800 || *size > 0x7ff)
	{
	  error (EXIT_SUCCESS, 0, _("\
stack size ``%s'' out of range: overflow in number"), optarg);
	  return -1;
	}
      *size <<= 20;
    }

  /* Sanity check for 64-bit machines.  */
  if (sizeof (long) > 4
      && ((*size) > 0x7fffffff
	  || (*size) < -0x80000000L))
    {
      error (EXIT_SUCCESS, 0, _("\
stack size ``%s'' out of range: overflow in number"), optarg);
      return -1;
    }

  return 0;
}

static int
#if PROTOTYPES
get_stackpos (struct mintbin_target* target)
#else
get_stackpos (target)
     struct mintbin_target* target;
#endif
{
  unsigned long stkpos = target->execp.g_stkpos;
  
  if (target->stksize_looked_up) 
    {
       if (stkpos <= 0)
	 {
	   if (filename_per_file)
	       error (EXIT_SUCCESS, 0, _("\
%s: file position of stack size not found"),
		      target->filename);
	       return -1;
	 }
    }
  else
    {
      /* Try to find the symbol _stksize.  */
      struct nlist* sym = NULL;
      size_t lastpos = 0;

      while (sym == NULL)
	{
	  sym = lookup_symbol (target, "__stksize", &lastpos);
	  if (sym == NULL)
	    break;

	  /* This is not yet sufficient.  It has to be a text or data
	     symbol and it must be defined.  */
	  if (((sym->n_type & N_EXT) == 0)
	      || ((sym->n_type & N_TYPE) != N_TEXT
		  && (sym->n_type & N_TYPE) != N_DATA
		  && (sym->n_type & N_TYPE) != N_WEAKT
		  && (sym->n_type & N_TYPE) != N_WEAKD))
	    sym = NULL;  /* I. e. continue the loop.  */
	}

      if (sym == NULL)
	{
	  if (filename_per_file)
	    error (EXIT_SUCCESS, 0, _("\
%s: cannot find symbol ``__stksize''"), target->filename);
	  else
	    error (EXIT_SUCCESS, 0, _("\
cannot find symbol ``__stksize''"));
	  return -1;
	}
      
      target->execp.g_stkpos = sym->n_value + 0x1c;
    }

  return 0;
}





