
# include the subdirs in the distribution
#DIST_SUBDIRS = translations utilities

incdir = @includedir@/licq

#TODO: convert this to something that works better
#but for now it should be sufficient
install-data-local:
	$(mkinstalldirs) $(incdir)
	@for i in $(srcdir)/*.h; do \
	  if test -f $$i; then \
	    echo "$(INSTALL_DATA) $$i $(incdir);"; \
	    $(INSTALL_DATA) $$i $(incdir); \
	  fi \
	done
#       $(mkinstalldirs) $(mandir)
#       $(INSTALL_DATA) licq.1 $(mandir)
