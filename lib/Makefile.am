## Process this file with automake to produce Makefile.in

# $Header$

noinst_LIBRARIES = libmb.a

EXTRA_DIST = alloca.c 

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I.. -I../intl -I$(top_srcdir)/intl
DEFS = @DEFS@

libmb_a_SOURCES = cplus-dem.c getopt.c getopt1.c xmalloc.c xstrdup.c error.c

libmb_a_LIBADD = @ALLOCA@ @LIBOBJS@

libmb_a_DEPENDENCIES = $(libmb_a_LIBADD)

noinst_HEADERS = ansidecl.h demangle.h error.h getopt.h libmb.h \
obstack.h
