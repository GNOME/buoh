SUBDIRS = icons

install-data-local: install-schemas update-icon-cache
uninstall-local: uninstall-schemas

#
# GConf schema
#

if WITH_GCONF
@INTLTOOL_SCHEMAS_RULE@

schema_in_files = buoh.schemas.in
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

install-schemas: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $(schema_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
			$(GCONFTOOL) \
			--makefile-install-rule \
			$(top_builddir)/data/$$p >&1 > /dev/null; \
	done \
	fi
endif

uninstall-schemas: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $^ ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		$(GCONFTOOL) \
		--makefile-uninstall-rule \
		$(top_builddir)/data/$$p; \
	done \
	fi
endif
	
else

install-schemas:
uninstall-schemas:

endif

comicsdir = $(comics_dir)

#
# UI descriptions
#

uidir = $(ui_dir)
ui_DATA = buoh-ui.xml

comics_DATA = comics.xml

#
# GTK icon cache
#

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache

update-icon-cache:
	if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

#
# Desktop file
#

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES = buoh.desktop.in
DESKTOP_FILES = $(DESKTOP_IN_FILES:.desktop.in=.desktop)

#
# Extra files to be included in the tarball
#

EXTRA_DIST =                   	\
	$(comics_DATA)		\
	$(ui_DATA)		\
	$(schema_in_files)      \
	$(DESKTOP_IN_FILES)

#
# Clean up properly
#

DISTCLEANFILES = \
	$(DESKTOP_FILES)	\
	$(schema_DATA)

