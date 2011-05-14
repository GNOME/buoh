SUBDIRS = icons

install-data-local: update-icon-cache

#
# GSetting schema
#

gsettings_SCHEMAS = org.gnome.buoh.gschema.xml

gsettings_ENUM_NAMESPACE = org.gnome.buoh
gsettings_ENUM_FILES = $(top_srcdir)/src/buoh-settings.h

@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

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

EXTRA_DIST =           	        	\
	$(comics_DATA)			\
	$(ui_DATA)			\
	$(gsettings_SCHEMAS)		\
	$(DESKTOP_IN_FILES)

#
# Clean up properly
#

DISTCLEANFILES = \
	$(DESKTOP_FILES)	\
	$(gsettings_SCHEMA)

