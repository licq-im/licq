top_srcdir = @top_srcdir@
prefix = @prefix@
exec_prefix = @exec_prefix@
srcdir = @srcdir@

bindir = @bindir@
sharedir = @datadir@/licq/

pkgdatadir = $(datadir)/@PACKAGE@

install-data-local:
	@if test "$(have_kde)" = "yes"; then \
		appsdir="`$(KDE_CONFIG) --install xdgdata-apps`"; \
		if test "$$appsdir" = "NONE"; then \
			appsdir="`$(KDE_CONFIG) --install apps`/Internet"; \
		fi; \
		appsdir=`echo $$appsdir | sed -e 's#$${datadir}#$(datadir)#'`; \
		$(mkinstalldirs) $(DESTDIR)$$appsdir; \
		$(INSTALL_DATA) $(srcdir)/licq.desktop $(DESTDIR)$$appsdir; \
	else \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/applications; \
		$(INSTALL_DATA) $(srcdir)/licq.desktop $(DESTDIR)$(datadir)/applications; \
	fi
	$(mkinstalldirs) $(DESTDIR)$(sharedir)
	$(mkinstalldirs) $(DESTDIR)$(sharedir)/qt-gui/locale
	@for j in $(srcdir)/qt-gui/skin.*; do \
	  jj="qt-gui/`basename $$j`" ; \
	  echo "Installing $$jj"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$jj; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	       $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$jj; \
	    fi \
	  done; \
	done
	@for j in $(srcdir)/qt-gui/dock.*; do \
	  jj="qt-gui/`basename $$j`" ; \
	  echo "Installing dock theme $$jj"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$jj; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$jj; \
	    fi \
	  done; \
	done
	@for j in $(srcdir)/qt-gui/icons.*; do \
	  jj="qt-gui/`basename $$j`" ; \
	  echo "Installing icons $$jj"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$jj; \
	  for i in $$j/*; do \
			if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$jj; \
			fi \
	  done; \
	done
	@for j in $(srcdir)/qt-gui/extended.icons.*; do \
	  jj="qt-gui/`basename $$j`" ; \
	  echo "Installing extended icons $$jj"; \
	  $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$jj; \
	  for i in $$j/*; do \
	    if test -f $$i; then \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$jj; \
	    fi \
	  done; \
	done
	$(mkinstalldirs) $(DESTDIR)$(sharedir)/qt-gui/emoticons
	@for j in `echo $(srcdir)/qt-gui/emoticons/*|sed s#$(srcdir)/qt-gui/emoticons/CVS##g`; do \
	  jj="qt-gui/emoticons/`basename $$j`" ; \
	  if test -d $$j; then \
	    echo "Installing emoticon $$jj"; \
	    $(mkinstalldirs) $(DESTDIR)$(sharedir)/$$jj; \
	    for i in $$j/*; do \
	      if test -f $$i; then \
	        $(INSTALL_DATA) $$i $(DESTDIR)$(sharedir)/$$jj; \
	      fi \
	    done; \
	  else \
	    $(INSTALL_DATA) $$j $(DESTDIR)$(sharedir)/qt-gui/emoticons/`basename $$j`; \
	  fi; \
	done
