bindir = @bindir@

#bin_PROGRAMS = beep

#beep_SOURCES = beep.c

#TODO: convert this to something that works better
#but for now it should be sufficient
install-data-local:
	$(mkinstalldirs) $(bindir)
	@for i in $(srcdir)/viewurl-*.sh; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_PROGRAM) $$i $(bindir);"; \
	    $(INSTALL_PROGRAM) $$i $(bindir); \
	  fi \
	done

