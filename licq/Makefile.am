
# don't warn about missing AUTHORS, NEWS, ChangeLog .. files
AUTOMAKE_OPTIONS = foreign 1.4

SUBDIRS = share src include bin po

EXTRA_DIST = admin/config.rpath  admin doc upgrade README-1.2.0 README.FREEBSD README.OPENSSL \
	 README.ICS post-install acinclude.m4.in plugins

$(top_srcdir)/acinclude.m4: $(top_srcdir)/acinclude.m4.in $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat $(top_srcdir)/acinclude.m4.in $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in > acinclude.m4

install-data-local:
	@cat $(srcdir)/post-install

uninstall-local: 
	-rm -rf $(libdir)/licq
	-rm -rf $(prefix)/share/licq
