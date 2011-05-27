/* cstrip.c -- copy GEMDOS-executable, massaging it.
   Copyright (C) 1999 Guido Flohr <gufl0000@stud.uni-sb.de>.

   This file is part of MiNTBin.

   MiNTBin is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   MiNTBin is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* $Id$ */

/* This file is partly inspired by objcopy.c from the GNU binutils.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# ifdef HAVE_STRING_H
#  include <string.h>
# else
#  ifdef HAVE_STRINGS_H
#   include <strings.h>
#  endif
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

#include <sys/stat.h>

#ifndef S_IRWXU
# define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif

#ifndef S_IRWXG
# define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif

#ifndef S_IRWXO
# define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#endif

#ifdef HAVE_GOOD_UTIME_H
#include <utime.h>
#else /* ! HAVE_GOOD_UTIME_H */
#ifdef HAVE_UTIMES
#include <sys/time.h>
#endif /* HAVE_UTIMES */
#endif /* ! HAVE_GOOD_UTIME_H */

#include <mint/a.out.h>
#include <mint/prg-out.h>

#include <libmb.h>

#include "targets.h"
#include "symbols.h"

#include "system.h"

/* A list of symbols to explicitly strip out, or to keep.  A linked
   list is good enough for a small number from the command line, but
   this will slow things down a lot if many symbols are being
   deleted. */

struct symlist
{
  const char *name;
  struct symlist *next;
};

static void usage PARAMS ((FILE *, int));
static int is_specified_symbol PARAMS ((const char *, struct symlist *));
static void add_specific_symbol PARAMS ((const char*, struct symlist**));
static int smart_rename PARAMS ((const char *, const char *));
static void set_times PARAMS ((const char *, const struct stat *));
static unsigned long filter_symbols PARAMS ((struct mintbin_target*,
					     struct mintbin_target*,
					     struct nlist*, struct nlist*,
					     unsigned long symcount));
static unsigned long copy_bytes PARAMS ((struct mintbin_target* to,
					 struct mintbin_target* from,
					 unsigned long bytes));
static void copy_file PARAMS ((const char*, const char*));
static void copy_object PARAMS ((struct mintbin_target*,
                                 struct mintbin_target*));
static int simple_copy PARAMS ((const char*, const char*));

static int verbose;		        /* Print file and target names.  */
static int status = EXIT_SUCCESS;	/* Exit status.  */

enum strip_action
  {
    strip_undef,
    strip_none,			/* Don't strip.  */
    strip_unneeded,		/* Strip unnecessary symbols.  */
    strip_all			/* Strip all symbols.  */
  };

/* Which symbols to remove. */
static enum strip_action strip_symbols;

enum locals_action
  {
    locals_undef,
    locals_start_L,		/* discard locals starting with L */
    locals_all			/* discard all locals */
  };

/* List of symbols to strip and keep.  */

static struct symlist *strip_specific_list = NULL;
static struct symlist *keep_specific_list = NULL;

static int strip_trailing_garbage = 0;

/* Options to handle if running as "strip".  */

static struct option long_options[] =
{
  {"help", no_argument, 0, 'h'},
  {"keep-symbol", required_argument, 0, 'K'},
  {"preserve-dates", no_argument, 0, 'p'},
  {"strip-all", no_argument, 0, 's'},
  {"strip-unneeded", no_argument, 0, 'u'},
  {"strip-symbol", required_argument, 0, 'N'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {"trailing-garbage", no_argument, &strip_trailing_garbage, 1},
  {0, no_argument, 0, 0}
};

extern char* make_tempname PARAMS ((char* filename));

char *program_name;

/* The number of bytes to copy at once.  */
#define COPY_BUF 8192
char buf[COPY_BUF];

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
  fputs (_("Strip old-style GEMDOS executables.\n"),
	 stream);
  fputs (_("\n\
Mandatory arguments to long options are mandatory to short options too.\n"),
	 stream);
  fputs (_("\
  -s, --strip-all                remove all symbols\n\
  -u, --strip-unneded            remove all unneeded symbols\n\
  -K, --keep-symbol=SYMBOLNAME   do not strip SYMBOLNAME\n\
  -N, --strip-symbol=SYMBOLNAME  remove SYMBOLNAME\n\
  -o, --output-file=OUTPUTFILE   put stripped output into OUTPUTFILE\n\
  -p, --preserve-dates           preserve the access and modification\n\
                                 dates of the file\n\
  -v, --verbose                  verbose: list all files modified\n"),
	 stream);
  fputs (_("\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n"),
	 stream);

  fputs (_("\n\
By default the program strips all unneeded symbols, that are all symbols\n\
except `__stksize' and `__inital_stack' which may be needed by programs\n\
that manipulate the stack size like the program `stack' from MiNTBin.\n\
If the object to be stripped is an executable in the extended MiNT\n\
format linked with the option `--traditional' it is not necessary to keep\n\
the above mentioned special symbols.\n\
\n\
The options `--keep-symbol' and `--strip-symbol' override all internal\n\
assumptions about which symbols to keep and which to strip.  If a symbol\n\
is specified both as being to keep and to strip, it is kept.\n\
\n\
Only old-style GEMDOS executables or MiNT executables with a DRI\n\
symbol-table (linked with the option `--traditional-format') are recognized.\n\
Use the program `strip' for all other formats.\n"), stream);
  fputs (_("Report bugs to <gufl0000@stud.uni-sb.de>.\n"), stream);

  exit (status);
}

/* Add a symbol to strip_specific_list.  */

static void 
add_specific_symbol (name, list)
     const char *name;
     struct symlist **list;
{
  struct symlist *tmp_list;

  tmp_list = (struct symlist *) xmalloc (sizeof (struct symlist));
  tmp_list->name = name;
  tmp_list->next = *list;
  *list = tmp_list;
}

/* See whether a symbol should be stripped or kept based on
   strip_specific_list and keep_symbols.  */

static int
is_specified_symbol (name, list)
     const char *name;
     struct symlist *list;
{
  struct symlist *tmp_list;

  for (tmp_list = list; tmp_list; tmp_list = tmp_list->next)
    {
      if (strcmp (name, tmp_list->name) == 0)
	return 1;
    }
  return 0;
}

/* Choose which symbol entries to copy; put the result in OSYMS.
   We don't copy in place, because that confuses the relocs.
   Return the number of symbols to print.  */

static unsigned long
filter_symbols (atarget, otarget, from, to, symcount)
     struct mintbin_target* atarget;
     struct mintbin_target* otarget;
     register struct nlist* from;
     register struct nlist* to;
     unsigned long symcount;
{
  unsigned long src_count = 0;
  unsigned long dst_count = 0;
  int keep_default = strip_symbols == strip_all ? 0 : 1;
  
  for (; src_count < symcount; src_count++)
    {
      struct nlist* sym = from + src_count;
      const char *name = sym->n_un.n_name;
      int keep = keep_default;

      if (keep && !is_specified_symbol (name, strip_specific_list))
	keep = 0;
      if (!keep && is_specified_symbol (name, keep_specific_list))
	keep = 1;

      /* Now for the special symbols __stksize and __initial_stksize.  */
      if (strip_symbols == strip_unneeded
          && keep_specific_list == NULL
          && atarget->format == oldstyle_prg_target)
        {
          if (strcmp (name, "__stksize") == 0)
            keep = 1;
          else if (strcmp (name, "__initial_stksize") == 0)
            keep = 1;
        }
        
      /* This reveals a design flaw in our mini bfd lib.  The symtab
         member would better be an array of pointers so that we don't
         have to copy the memory but copy the pointers instead.  */
      if (keep)
	to[dst_count++] = *sym;
    }

  return dst_count;
}

/* Copy object file ITARGET onto OTARGET.  */

static void
copy_object (itarget, otarget)
     struct mintbin_target* itarget;
     struct mintbin_target* otarget;
{
  long int chunk;
  unsigned long i;
  size_t rest;

  if (verbose)
    printf (_("copy from %s(%s) to %s(%s)\n"),
	    itarget->filename, 
	    itarget->format == prg_target ? "a.out-mintprg" : "gemdos",
	    otarget->filename,
	    otarget->format == prg_target ? "a.out-mintprg" : "gemdos");

  if (itarget->symtab != NULL)
    {
      otarget->symtab = xmalloc (itarget->symcount * sizeof (struct nlist));

      /* Copy the symbol table, obeying the keep and strip lists.  */
      otarget->symcount = filter_symbols (itarget, otarget, itarget->symtab,
					  otarget->symtab, itarget->symcount);
    }

  /* Copy the header, text and data.  */
  chunk = 28 + itarget->execp.g_text + itarget->execp.a_data;

  if (lseek (itarget->desc, 0, SEEK_SET) != 0)
    {
      (void) close (otarget->desc);
      otarget->desc = -1;  /* We use this to report failure.  */
      error (EXIT_SUCCESS, errno, _("\
%s: seek error"), itarget->filename);
      return;
    }
  if (copy_bytes (otarget, itarget, chunk) != chunk)
    return;

  /* Write the symbol table.  */
  otarget->execp.g_syms = 0;
  for (i = 0; i < otarget->symcount; i++)
    {
      char outsym[28];
      struct nlist* sym = otarget->symtab + i;
      char* name = sym->n_un.n_name;
      size_t len = strlen (name);
      strcpy (outsym, name);
      /* The symbol type.  */
      outsym[8] = (sym->n_desc >> 8) & 0xff;
      outsym[9] = sym->n_desc & 0xff;
      /* The symbol value.  */
      outsym[10] = (sym->n_value >> 24) & 0xff;
      outsym[11] = (sym->n_value >> 16) & 0xff;
      outsym[12] = (sym->n_value >> 8) & 0xff;
      outsym[13] = sym->n_value & 0xff;
      chunk = 14;
      if (len > 8)
        {
          memcpy (outsym + 14, name + 8, 14);
	  chunk += 14;
          outsym[9] |= A_LNAM;
        }
      otarget->execp.g_syms += chunk;
      if (write (otarget->desc, outsym, chunk) != chunk)
	{
	  error (EXIT_SUCCESS, errno, _("%s: write error"), otarget->filename);
	  (void) close (otarget->desc);
	  otarget->desc = -1;
	  return;
	}
    }  
  
  /* Copy the rest of the file.  */
  if (lseek (itarget->desc, 28 + itarget->execp.g_text +
	     itarget->execp.a_data + itarget->execp.g_syms, SEEK_SET) 
	         != 28 + itarget->execp.g_text + itarget->execp.a_data
	          + itarget->execp.g_syms)
    {
      (void) close (otarget->desc);
      otarget->desc = -1;  /* We use this to report failure.  */
      error (EXIT_SUCCESS, errno, _("\
%s: seek error"), itarget->filename);
      return;
    }
  
  rest = itarget->size - 28 - itarget->execp.g_text - itarget->execp.a_data
    - itarget->execp.g_syms;
  if (rest > itarget->size)
    {
      /* Bogus header data.  */
      error (EXIT_SUCCESS, 0, _("%s: header corrupted"), itarget->filename);
      (void) close (otarget->desc);
      otarget->desc = -1;
      return;
    }
  if (copy_bytes (otarget, itarget, rest) != rest)
    return;

  /* Finally correct the header.  */
  if (lseek (otarget->desc, 14, SEEK_SET) != 14)
    {
      (void) close (otarget->desc);
      otarget->desc = -1;  /* We use this to report failure.  */
      error (EXIT_SUCCESS, errno, _("\
%s: seek error"), itarget->filename);
      return;
    }
  buf[0] = (otarget->execp.g_syms >> 24) & 0xff;
  buf[1] = (otarget->execp.g_syms >> 16) & 0xff;
  buf[2] = (otarget->execp.g_syms >> 8) & 0xff;
  buf[3] = otarget->execp.g_syms & 0xff;
  if (write (otarget->desc, buf, 4) != 4)
    {
      (void) close (otarget->desc);
      otarget->desc = -1;  /* We use this to report failure.  */
      error (EXIT_SUCCESS, errno, _("\
%s: seek error"), otarget->filename);
      return;
    }
}

/* Copy BYTES from target FROM to target TO.  Return
   the number of bytes copied.  */
static unsigned long
copy_bytes (to, from, bytes)
     struct mintbin_target* to;
     struct mintbin_target* from;
     unsigned long bytes;
{
  unsigned long written = 0;
  while (written < bytes)
    {
      unsigned long chunk = bytes - written > COPY_BUF ?
	COPY_BUF : bytes - written;
      if (read (from->desc, buf, chunk) != chunk)
	{
	  (void) close (to->desc);
	  to->desc = -1;  /* We use this to report failure.  */
	  error (EXIT_SUCCESS, errno, _("\
%s: read error"), from->filename);
	  return 0;
	}
      if (write (to->desc, buf, chunk) != chunk)
	{
	  (void) close (to->desc);
	  to->desc = -1;  /* We use this to report failure.  */
	  error (EXIT_SUCCESS, errno, _("\
%s: write error"), to->filename);
	  return 0;
	}
      written += chunk;
    }
  return written;
}

/* The top-level control.  */

static void
copy_file (input_filename, output_filename)
     const char *input_filename;
     const char *output_filename;
{
  struct mintbin_target* itarget;
  struct mintbin_target* otarget;

  /* To allow us to do "cstrip *" without dying on the first
     non-object file, failures are nonfatal.  */

  itarget = open_target (input_filename, O_RDONLY);
  
  if (itarget == NULL)
    {
      status = EXIT_FAILURE;
      return;
    }

  if (!(itarget->format == oldstyle_prg_target ||
	(itarget->format == prg_target
	 && itarget->execp.g_symbol_format == _DRI_FORMAT)))
    {
      error (EXIT_SUCCESS, 0, _("\
%s: file format not recognized, use `strip' instead"),
	     itarget->filename);
      status = EXIT_FAILURE;
      (void) close_target (itarget);
      return;
    }

  /* Now slurp in the symbol table.  */
  if (strip_symbols != strip_all && swap_symtab_in (itarget, 1) != 0)
      {
        error (EXIT_SUCCESS, 0, _("\
%s: error reading symbol table, use `%s --strip-all' to force stripping"),
	       itarget->filename, program_name);
	(void) close_target (itarget);
	status = EXIT_FAILURE;
	return;
      }
  
  otarget = xmalloc (sizeof *otarget);
  (void) memset (otarget, 0, sizeof *otarget);
  otarget->filename = output_filename;
  otarget->desc = creat (output_filename, S_IRWXU | S_IRWXG | S_IRWXO);
  if (otarget->desc < 0)
    {
      status = EXIT_FAILURE;
      error (EXIT_SUCCESS, errno, _("can\'t create output file `%s'"),
	     otarget->filename);
      (void) close_target (itarget);
      return;
    }
  
  /* Initialize as much of OTARGET as possible.  */
  otarget->format = itarget->format;
  otarget->name = itarget->name;
  otarget->execp = itarget->execp;
  otarget->header = NULL;
  otarget->header_bytes = 0;
  otarget->symtab = NULL;
  otarget->strtab = NULL;
  otarget->symcount = 0;

  copy_object (itarget, otarget);
  
  if (otarget->desc >= 0)
    {
      if (close_target (otarget) != 0)
        {
          status = EXIT_FAILURE;
          error (EXIT_SUCCESS, errno, _("can\'t close output file `%s'"),
	         otarget->filename);
        }
  }
  
  (void) close_target (itarget);
}


/* Copy file FROM to file TO, performing no translations.
   Return 0 if ok, -1 if error.  */
static int
simple_copy (from, to)
     const char *from;
     const char *to;
{
  int fromfd, tofd, nread;
  int saved;

  fromfd = open (from, O_RDONLY);
  if (fromfd < 0)
    return -1;
  tofd = creat (to, S_IRWXU | S_IRWXG | S_IRWXO);
  if (tofd < 0)
    {
      saved = errno;
      close (fromfd);
      errno = saved;
      return -1;
    }
  while ((nread = read (fromfd, buf, sizeof buf)) > 0)
    {
      if (write (tofd, buf, nread) != nread)
	{
	  saved = errno;
	  close (fromfd);
	  close (tofd);
	  errno = saved;
	  return -1;
	}
    }
  saved = errno;
  close (fromfd);
  close (tofd);
  if (nread < 0)
    {
      errno = saved;
      return -1;
    }
  return 0;
}

#ifndef S_ISLNK
#ifdef S_IFLNK
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(m) 0
#define lstat stat
#endif
#endif

/* Rename FROM to TO, copying if TO is a link.
   Assumes that TO already exists, because FROM is a temp file.
   Return 0 if ok, -1 if error.  */

static int
smart_rename (from, to)
     const char *from;
     const char *to;
{
  struct stat s;
  int ret = 0;

  if (lstat (to, &s))
    return -1;

#if defined (_WIN32) && !defined (__CYGWIN32__)
  /* Win32, unlike unix, will not erase `to' in `rename(from, to)' but
     fail instead.  Also, chown is not present.  */

  if (from != NULL)
    {
      if (stat (to, &s) == 0)
        remove (to);

      ret = rename (from, to);
      if (ret != 0)
        {
          error (EXIT_SUCCESS, errno, _("error renaming `%s' to `%s'"), 
                 from, to);
          unlink (from);
        }
  }
#else
  /* Use rename only if TO is not a symbolic link and has
     only one hard link.  */
  if (!S_ISLNK (s.st_mode) && s.st_nlink == 1)
    {
      if (from != NULL)
        ret = rename (from, to);
      if (ret == 0)
	{
	  /* Try to preserve the permission bits and ownership of TO.
             First get the mode right except for the setuid bit.  Then
             change the ownership.  Then fix the setuid bit.  We do
             the chmod before the chown because if the chown succeeds,
             and we are a normal user, we won't be able to do the
             chmod afterward.  We don't bother to fix the setuid bit
             first because that might introduce a fleeting security
             problem, and because the chown will clear the setuid bit
             anyhow.  We only fix the setuid bit if the chown
             succeeds, because we don't want to introduce an
             unexpected setuid file owned by the user running objcopy.  */
	  chmod (to, s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
	  if (chown (to, s.st_uid, s.st_gid) >= 0)
	    chmod (to, s.st_mode & (S_ISUID | S_IRWXU | S_IRWXG | S_IRWXO));
	}
      else
	{
	  error (EXIT_SUCCESS, errno, _("error renaming `%s' to `%s'"), 
	         from, to);
	  unlink (from);
	}
    }
  else
    {
      if (from != NULL)
        {
          ret = simple_copy (from, to);
          if (ret != 0)
  	    {
	      error (EXIT_SUCCESS, errno, _("error copying `%s' to `%s'"), 
	             from, to);
	    }
          unlink (from);
        }
    }
#endif /* _WIN32 && !__CYGWIN32__ */

  return ret;
}

/* Set the times of the file DESTINATION to be the same as those in
   STATBUF.  */

static void
set_times (destination, statbuf)
     const char *destination;
     const struct stat *statbuf;
{
  int result;

  {
#ifdef HAVE_GOOD_UTIME_H
    struct utimbuf tb;

    tb.actime = statbuf->st_atime;
    tb.modtime = statbuf->st_mtime;
    result = utime (destination, &tb);
#else /* ! HAVE_GOOD_UTIME_H */
#ifndef HAVE_UTIMES
    long tb[2];

    tb[0] = statbuf->st_atime;
    tb[1] = statbuf->st_mtime;
    result = utime (destination, tb);
#else /* HAVE_UTIMES */
    struct timeval tv[2];

    tv[0].tv_sec = statbuf->st_atime;
    tv[0].tv_usec = 0;
    tv[1].tv_sec = statbuf->st_mtime;
    tv[1].tv_usec = 0;
    result = utimes (destination, tv);
#endif /* HAVE_UTIMES */
#endif /* ! HAVE_GOOD_UTIME_H */
  }

  if (result != 0)
    {
      error (EXIT_SUCCESS, errno, _("error setting timestamp of `%s'"), 
             destination);
    }
}

int
main (argc, argv)
     int argc;
     char *argv[];
{
  int show_version = 0;
  int preserve_dates = 0;
  int c, i;
  char *output_file = NULL;

  strip_symbols = strip_undef;

  if (argv != NULL)
    program_name = argv[0];

  if (program_name == NULL || program_name[0] == '\0')
    program_name = "cstrip";
  
#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((c = getopt_long (argc, argv, "K:N:o:spuVv",
			   long_options, (int *) 0)) != EOF)
    {
      switch (c)
	{
	case 's':
	  strip_symbols = strip_all;
	  break;
	case 'K':
	  add_specific_symbol (optarg, &keep_specific_list);
	  break;
	case 'N':
	  add_specific_symbol (optarg, &strip_specific_list);
	  break;
	case 'o':
	  output_file = optarg;
	  break;
	case 'p':
	  preserve_dates = 1;
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'V':
	  show_version = 1;
	  break;
	case 0:
	  break;		/* we've been given a long option */
	case 'h':
	  usage (stdout, EXIT_SUCCESS);
	default:
	  usage (stderr, EXIT_FAILURE);
	}
    }

  if (show_version)
    print_version (program_name);

  /* Default is to strip unneeded symbols.  */
  if (strip_symbols == strip_undef
      && strip_specific_list == NULL
      && keep_specific_list == NULL)
    {
      strip_symbols = strip_unneeded;
    }

  i = optind;
  if (i == argc
      || (output_file != NULL && (i + 1) < argc))
    usage (stderr, 1);

  for (; i < argc; i++)
    {
      int hold_status = status;
      struct stat statbuf;
      char *tmpname;

      if (preserve_dates)
	{
	  if (stat (argv[i], &statbuf) < 0)
	    {
              error (EXIT_SUCCESS, errno, _("%s: stat error"), argv[i]);
	      continue;
	    }
	}

      if (output_file != NULL)
	tmpname = output_file;
      else
	tmpname = make_tempname (argv[i]);
      status = EXIT_SUCCESS;

      copy_file (argv[i], tmpname);
      if (status == 0)
	{
	  if (preserve_dates)
	    set_times (tmpname, &statbuf);
	  if (output_file == NULL)
	    smart_rename (tmpname, argv[i]);
	  else
	    smart_rename (NULL, output_file);
	  status = hold_status;
	}
      else
	unlink (tmpname);
      if (output_file == NULL)
	free (tmpname);
    }

  return status;
}
