## Process this file with automake to produce Makefile.in.

# $Id$

AUTOMAKE_OPTIONS = gnits

mintincludedir = $(includedir)/mint

EXTRA_DIST = a.out.h prg-out.h

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(mintincludedir)
	@list='a.out.h prg-out.h'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(mintincludedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(mintincludedir)/$$p; \
	done
	if test x$(DESTDIR)$(nativeincludedir) != x$(DESTDIR); then \
          $(mkinstalldirs) $(DESTDIR)$(nativeincludedir)/mint || exit 1; \
          list='a.out.h prg-out.h'; for header in $$list; do \
            ln $(DESTDIR)$(mintincludedir)/$$header $(DESTDIR)$(nativeincludedir)/mint/$$header >/dev/null 2>/dev/null \
	    || @INSTALL_DATA@ $$header $(DESTDIR)$(nativeincludedir)/mint ;\
          done; \
	fi

uninstall-local:
	@$(NORMAL_UNINSTALL)
	list='a.out.h prg-out.h'; for p in $$list; do \
	  rm -f $(DESTDIR)$(mintincludedir)/$$p; \
	done
	if test x$(nativeincludedir) != x$(DESTDIR); then \
	  list='a.out.h prg-out.h'; for header in $$list; do \
	    rm -f $(DESTDIR)$(nativeincludedir)/mint/$$header; \
          done; \
	fi
