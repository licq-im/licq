top_srcdir = @top_srcdir@
prefix = @prefix@
exec_prefix = @exec_prefix@

bindir = @bindir@
sharedir = @datadir@/licq/

pkgdatadir = $(datadir)/@PACKAGE@

install-data-local:
	$(mkinstalldirs) $(sharedir)
	$(mkinstalldirs) $(sharedir)/qt-gui/locale
	@for j in qt-gui/skin.*; do \
	  echo "Installing skin $$j"; \
	  $(mkinstalldirs) $(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(sharedir)/$$j; \
	    fi \
	  done; \
	done
	@for j in qt-gui/dock.*; do \
	  echo "Installing dock theme $$j"; \
	  $(mkinstalldirs) $(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(sharedir)/$$j; \
	    fi \
	  done; \
	done
	@for j in qt-gui/icons.*; do \
	  echo "Installing icons $$j"; \
	  $(mkinstalldirs) $(sharedir)/$$j; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(sharedir)/$$j; \
			fi \
	  done; \
	done

