$(top_srcdir)/acinclude.m4: $(top_srcdir)/acinclude.m4.in $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat acinclude.m4.in admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

# don't warn about missing AUTHORS, NEWS, ChangeLog .. files
AUTOMAKE_OPTIONS = foreign 1.4

SUBDIRS = src
