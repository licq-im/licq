localedir = $(prefix)/share/licq/qt-gui/locale

%.qm: $(top_srcdir)/po/%.ts
	$(QT_LRELEASE) -verbose $< -qm $@

# The row below transforms $(top_srcdir)/po/*.ts into *.qm
# forcing all .ts files in the po dir to be compiled into .qm files.
all-local: $(addsuffix .qm, $(basename $(notdir $(wildcard $(top_srcdir)/po/*.ts))))

clean-local:
	-rm -f $(top_builddir)/po/*.qm
	-rm -f $(top_builddir)/po/template.ts

# We clean the srcdir as well just to be sure no qm-files are left.
maintainer-clean-local:
	-rm -f $(top_srcdir)/po/*.qm
	-rm -f $(top_srcdir)/po/template.ts

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(localedir)
	@for file in $(wildcard $(top_builddir)/po/*.qm); do \
		echo " $(INSTALL_DATA) $$file $(DESTDIR)$(localedir)/`basename $$file`"; \
		$(INSTALL_DATA) $$file $(DESTDIR)$(localedir)/`basename $$file`; \
	done
	cd $(DESTDIR)$(localedir) && rm -f cs.qm
	cd $(DESTDIR)$(localedir) && ln -s cs_CZ.qm cs.qm

uninstall-local:
	-rm -f $(DESTDIR)$(localedir)/*.qm

merge:
	$(QT_LUPDATE) $(LUPDATE_OPTS) $(top_srcdir)/src/*.cpp $(top_srcdir)/src/*.h -ts $(top_srcdir)/po/*.ts

clean-merge: LUPDATE_OPTS=-noobsolete
clean-merge: merge

template:
	$(QT_LUPDATE) $(top_srcdir)/src/*.cpp $(top_srcdir)/src/*.h -ts $(top_builddir)/po/template.ts
