SUBDIRS = icons

@INTLTOOL_SCHEMAS_RULE@

comicsdir = $(comics_dir)
uidir = $(ui_dir)
schemadir = $(GCONF_SCHEMA_FILE_DIR)

comics_DATA = comics.xml
ui_DATA = buoh-ui.xml

schema_in_files = buoh.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

Applicationsdir = $(datadir)/applications
Applications_in_files = buoh.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	   for p in $(schema_DATA) ; do \
	   GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p >&1 > /dev/null; \
	   done \
	fi
endif
	if test -z "$(DESTDIR)"; then \
	   echo "Updating Gtk icon cache."; \
	   $(gtk_update_icon_cache); \
	else \
	   echo "*** Icon cache not updated.  After install, run this:"; \
	   echo "***   $(gtk_update_icon_cache)"; \
	fi


DISTCLEANFILES =	 	\
		buoh.desktop    \
		*.bak		\
		*.gladep	\
		$(schema_DATA)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                    	\
		$(comics_DATA)		\
		$(ui_DATA)		\
		$(schema_in_files)      \
		$(Applications_in_files)


