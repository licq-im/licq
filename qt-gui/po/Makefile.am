#HACK, FIXME in a more clean manner
localedir = $(prefix)/share/licq/qt-gui/locale

TRANSLATIONS = de es ru_RU.KOI8-R

merge: template
	@for i in *.po; do \
	  $(QT_MERGETR) $$i template.po; \
	done

install-data-local:
	$(mkinstalldirs) $(localedir)
	@rm -f *.qm
	@for i in $(TRANSLATIONS); do \
	  $(QT_MSG2QM) $$i.po $$i.qm >/dev/null 2>&1; \
	done
	@for i in *.qm; do \
	  echo " $(INSTALL_DATA) $$i $(localedir)/$$i"; \
	  $(INSTALL_DATA) $$i $(localedir)/$$i; \
	  rm -f $$i; \
	done

template:
	cd $(top_srcdir)/src && $(QT_FINDTR) *.cpp *.h \
		> $(top_srcdir)/po/template.po

