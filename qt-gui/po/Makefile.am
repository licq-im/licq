#HACK, FIXME in a more clean manner
#
#sorry.. another hack for russian people with non-koi8 charset...
#will be removed after qt2.1 releases

localedir = $(DESTDIR)$(prefix)/share/licq/qt-gui/locale

all-local:
	@if test -x /usr/bin/test; then \
	    nt_test="/usr/bin/test"; \
	  else \
	    nt_test=`which test`; \
	fi ;\
	for i in $(srcdir)/*.po; do \
	  f=`basename $$i .po`; \
	  if test ! -r $(top_builddir)/po/$$f.qm || \
	    $${nt_test} $(srcdir)/$$f.po -nt $(top_builddir)/po/$$f.qm; then \
	 	    if test "$$f" != "template"; then \
	        echo "$(QT_MSG2QM) $(srcdir)/$$f.po $(top_builddir)/po/$$f.qm"; \
	        $(QT_MSG2QM) $(srcdir)/$$f.po $(top_builddir)/po/$$f.qm; \
	      fi \
	  fi \
	done

clean-local:
	-rm -f $(top_builddir)/po/template.po

maintainer-clean-local:
	-rm -f $(top_builddir)/po/*.qm

merge: template
	@for i in $(srcdir)/*.po; do \
	  if test "$$i" != "$(srcdir)/template.po"; then \
	    echo "Merging $$i"; \
		  $(QT_MERGETR) $$i template.po; \
	  fi \
	done

install-data-local:
	$(mkinstalldirs) $(localedir)
	@for i in ./*.qm; do \
	  if test "$$i" != "template.qm"; then \
	    echo " $(INSTALL_DATA) $$i $(localedir)/$$i"; \
	    $(INSTALL_DATA) $$i $(localedir)/$$i; \
	  fi \
	done; \
	cd $(localedir) && rm -f cs.qm; \
	cd $(localedir) && ln -s cs_CZ.qm cs.qm

template: 
	cd $(top_srcdir)/src && $(QT_FINDTR) *.cpp *.h > $(top_builddir)/po/template.po
	cat addon.qt >>template.po

uninstall-local:
	-rm -f $(localedir)/*.qm
