## Process this file with automake to create Makefile.in

# $Id$

AM_CFLAGS = -Wall

datadir = $(prefix)/@DATADIRNAME@
localedir = $(datadir)/locale

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir) -I$(srcdir) \
-I$(top_srcdir)/lib -I../intl -I$(top_srcdir)/intl -I..
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

LDADD = ../lib/libmb.a

LIBS = $(INTLLIBS) @LIBS@

noinst_HEADERS = exec.h symbols.h targets.h

bin_PROGRAMS = arconv cnm csize cstrip flags mintbin stack symex

arconv_SOURCES = arconv.c targets.c version.c
cnm_SOURCES = cnm.c symbols.c targets.c version.c
csize_SOURCES = csize.c targets.c version.c
cstrip_SOURCES = cstrip.c symbols.c targets.c tempname.c version.c
flags_SOURCES = flags.c targets.c version.c
mintbin_SOURCES = mintbin.c targets.c version.c
stack_SOURCES = stack.c symbols.c targets.c version.c
symex_SOURCES = symex.c targets.c version.c

install-exec-local:
	@$(NORMAL_INSTALL)
	if test x$(DESTDIR)$(nativebindir) != x; then \
	  $(mkinstalldirs) $(DESTDIR)$(nativebindir) || exit 1; \
	  for prog in $(bin_PROGRAMS); do \
	    iprog=`echo $$prog | sed '$(transform)'`; \
            ln $(DESTDIR)$(bindir)/$$iprog $(DESTDIR)$(nativebindir)/m68k-atari-mint-$$iprog >/dev/null 2>/dev/null \
	      || $(INSTALL_PROGRAM) $$prog $(DESTDIR)$(nativebindir)/m68k-atari-mint-$$iprog ; \
	  done; \
        fi

uninstall-local:
	if test x$(nativebindir) != x; then \
	  for prog in $(bin_PROGRAMS); do \
	    iprog=`echo $$prog | sed '$(transform)'`; \
            rm -f $(DESTDIR)$(nativebindir)/m68k-atari-mint-$$iprog; \
	  done; \
        fi
