#HACK, FIXME in a more clean manner
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
	  if test "$$i" != "$(srcdir)/template.qm"; then \
	    echo " $(INSTALL_DATA) $$i $(localedir)/$$i"; \
	    $(INSTALL_DATA) $$i $(localedir)/$$i; \
          fi \
	done

template: $(top_srcdir)/src/*.cpp
	cd $(top_srcdir)/src && $(QT_FINDTR) *.cpp *.h > $(srcdir)/template.po

