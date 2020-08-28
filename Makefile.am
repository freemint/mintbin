## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = gnits dist-xz
ACLOCAL_AMFLAGS = -I m4

MAINT_CHARSET = latin1

SUBDIRS = lib include intl src po doc misc

EXTRA_DIST = README-alpha system.h

# Makefiles in the po directory are from on
# older version of gettext and try to move
# po files around in the source directory,
# but that is made read-only by newer versions
# of automake
DISTCHECK_CONFIGURE_FLAGS = --disable-nls
