bindir = @bindir@

#TODO: convert this to something that works better
#but for now it should be sufficient
install-data-local:
	$(mkinstalldirs) $(bindir)
	@for i in $(srcdir)/viewurl-netscape.sh; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(bindir);"; \
	    $(INSTALL_DATA) $$i $(bindir); \
	  fi \
	done

