
sharedir = $(DESTDIR)@datadir@/licq
plugindir = $(DESTDIR)@libdir@/licq
mandir = $(DESTDIR)@mandir@/man1

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
	@for i in $(srcdir)/sounds/*.wav; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/sounds;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/sounds; \
	  fi \
	done
	$(mkinstalldirs) $(sharedir)/sounds/fun
	@for i in $(srcdir)/sounds/fun/*; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/sounds/fun;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/sounds/fun; \
	  fi \
	done
	$(mkinstalldirs) $(sharedir)/sounds/icq
	@for i in $(srcdir)/sounds/icq/*; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(sharedir)/sounds/icq;"; \
	    $(INSTALL_DATA) $$i $(sharedir)/sounds/icq; \
	  fi \
	done
