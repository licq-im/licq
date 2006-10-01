AUTOMAKE_OPTIONS = foreign # 1.5 # dist-bzip2
SUBDIRS = src po

EXTRA_DIST = admin/mkinstalldirs  admin/config.rpath  licq_osd.conf GPL TODO README changelog ABOUT-NLS

$(top_srcdir)/acinclude.m4: $(top_srcdir)/acinclude.m4.in $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat acinclude.m4.in admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4
