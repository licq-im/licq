
# include the subdirs in the distribution
DIST_SUBDIRS = translations utilities sounds

sharedir = @datadir@/licq
plugindir = @libdir@/licq
mandir = @mandir@/man1

#TODO: convert this to something that works better
#but for now it should be sufficient
install-data-local:
	$(mkinstalldirs) $(plugindir)
	$(mkinstalldirs) $(sharedir)
	$(mkinstalldirs) $(sharedir)/utilities
	@for i in $(srcdir)/utilities/*; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/utilities;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/utilities; \
	  fi \
	done
	$(mkinstalldirs) $(sharedir)/translations
	@for i in $(srcdir)/translations/*; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/translations;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/translations; \
	  fi \
	done
	$(mkinstalldirs) $(sharedir)/sounds
	@for i in $(srcdir)/sounds/*; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/sounds;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/sounds; \
	  fi \
	done
#       $(mkinstalldirs) $(mandir)
#       $(INSTALL_DATA) licq.1 $(mandir)
