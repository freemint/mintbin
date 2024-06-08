## Process this file with automake to produce Makefile.in.

# $Id$

AUTOMAKE_OPTIONS = gnits

SUBDIRS = mint

include_HEADERS = a.out.h stab.h stab.def

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(includedir)
	for p in $(include_HEADERS); do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p; \
	done
	if test x$(DESTDIR)$(nativeincludedir) != x$(DESTDIR); then \
	  $(mkinstalldirs) $(DESTDIR)$(nativeincludedir) || exit 1; \
	  for header in $(include_HEADERS); do \
            ln $(DESTDIR)$(includedir)/$$header $(DESTDIR)$(nativeincludedir)/$$header >/dev/null 2>/dev/null \
	      || @INSTALL_DATA@ $$header $(DESTDIR)$(nativeincludedir); \
	  done; \
        fi

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for p in $(include_HEADERS); do \
	  rm -f $(DESTDIR)$(includedir)/$$p; \
	done
	if test x$(nativeincludedir) != x$(DESTDIR); then \
	  for header in $(include_HEADERS); do \
            rm -f $(DESTDIR)$(nativeincludedir)/$$header; \
	  done; \
        fi


