#HACK, FIXME in a more clean manner
#
#sorry.. another hack for russian people with non-koi8 charset...
#will be removed after qt2.1 releases

localedir = $(prefix)/share/licq/qt-gui/locale

all-local:
	@if test -x /usr/bin/test; then \
	  nt_test="/usr/bin/test"; \
        else \
          nt_test=`which test`; \
        fi ;\
	for i in $(srcdir)/*.po; do \
	  f=$(srcdir)/`basename $$i .po`; \
          if test ! -r $$f.qm || $${nt_test} $$f.po -nt $$f.qm; then \
 	    if test "$$f" != "$(srcdir)/template"; then \
	      echo "$(QT_MSG2QM) $$f.po $$f.qm"; \
	      $(QT_MSG2QM) $$f.po $$f.qm; \
            fi \
          fi \
        done

clean-local:
	-rm -f *.qm

merge: template
	@for i in $(srcdir)/*.po; do \
	  if test "$$i" != "$(srcdir)/template.po"; then \
  	    $(QT_MERGETR) $$i template.po; \
	  fi \
	done

install-data-local:
	$(mkinstalldirs) $(localedir)
	@cd $(srcdir) && for i in *.qm; do \
	  if test "$$i" != "template.qm"; then \
	    echo " $(INSTALL_DATA) $$i $(localedir)/$$i"; \
	    $(INSTALL_DATA) $$i $(localedir)/$$i; \
          fi \
	done; \
        cd $(localedir) && ln -sf ru_RU.KOI8-R.qm ru.qm; \
        cd $(localedir) && ln -sf cs_CZ.qm cs.qm

template: 
	cd $(top_srcdir)/src && $(QT_FINDTR) *.cpp *.h > $(top_srcdir)/po/template.po

