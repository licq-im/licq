top_srcdir = @top_srcdir@
prefix = @prefix@
exec_prefix = @exec_prefix@

bindir = @bindir@
sharedir = @datadir@/licq/

pkgdatadir = $(datadir)/@PACKAGE@

install-data-local:
	@if test "$(KDEDIR)" != ""; then \
	   if test -w "$(KDEDIR)"; then \
	     $(mkinstalldirs) $(DESTDIR)$(KDEDIR)/share/applnk/Internet; \
	     $(INSTALL_DATA) licq.desktop $(DESTDIR)$(KDEDIR)/share/applnk/Internet; \
	   else \
	     $(mkinstalldirs) $(HOME)/.kde/share/applnk/Internet; \
	     $(INSTALL_DATA) licq.desktop $(HOME)/.kde/share/applnk/Internet; \
	   fi  \
	fi
	$(mkinstalldirs) $(DESTDIR)$(sharedir)
	$(mkinstalldirs) $(DESTDIR)$(sharedir)/qt-gui/locale
	@for j in qt-gui/skin.*; do \
	  echo "Installing skin $$j"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$j; \
	    fi \
	  done; \
	done
	@for j in qt-gui/dock.*; do \
	  echo "Installing dock theme $$j"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$j; \
	    fi \
	  done; \
	done
	@for j in qt-gui/icons.*; do \
	  echo "Installing icons $$j"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$j; \
			fi \
	  done; \
	done

