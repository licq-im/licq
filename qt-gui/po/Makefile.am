#HACK, FIXME in a more clean manner
localedir = $(prefix)/share/licq/qt-gui/locale

all-local:
	@for i in $(srcdir)/*.po; do \
	  f=$(srcdir)/`basename $$i .po`; \
          if test ! -r $$f.qm || test $$f.po -nt $$f.qm; then \
	    echo "$(QT_MSG2QM) $$f.po $$f.qm"; \
	    $(QT_MSG2QM) $$f.po $$f.qm; \
          fi \
        done

clean-local:
	-rm -f *.qm

merge: template
	@for i in $(srcdir)/*.po; do \
	  $(QT_MERGETR) $$i template.po; \
	done

install-data-local:
	$(mkinstalldirs) $(localedir)
	@for i in $(srcdir)/*.qm; do \
	  echo " $(INSTALL_DATA) $$i $(localedir)/$$i"; \
	  $(INSTALL_DATA) $$i $(localedir)/$$i; \
	done

template:
	cd $(top_srcdir)/src && $(QT_FINDTR) *.cpp *.h \
		> $(top_srcdir)/po/template.po

