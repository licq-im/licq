$(top_srcdir)/acinclude.m4: $(top_srcdir)/acinclude.m4.in $(top_srcdir)/../../libtool.m4.in
	@cd $(top_srcdir) && cat $(top_srcdir)/acinclude.m4.in $(top_srcdir)/../../libtool.m4.in > acinclude.m4

# don't warn about missing AUTHORS, NEWS, ChangeLog .. files
AUTOMAKE_OPTIONS = foreign 1.4

SUBDIRS = share src po

EXTRA_DIST = doc

cvs:
	@if test ! -d CVS; then \
          echo "you don't have a toplevel CVS directory."; \
          echo "You most certainly didn't use cvs to get these sources."; \
          echo "But this function depends on cvs's informations."; \
          exit 1 ;\
        fi;\
        pwd=`pwd` ;\
        dirs=`find . -type d | grep -v CVS | sed -e "s#^./##"` ;\
        for i in $$dirs; do \
          if test ! -d $$pwd/$$i/CVS; then \
            echo "D $$i" ;\
            rm -rf $$pwd/$$i; \
            continue; \
          fi ;\
          cd $$pwd/$$i ;\
          for f in * .*; do \
            if test ! -d $$f; then \
              if grep "^/$$f/" CVS/Entries >/dev/null; then \
                a="b"; \
              else \
                echo "F $$i/$$f"; \
                rm -f $$pwd/$$i/$$f; \
              fi; \
            fi ; \
          done; \
        done

