@INTLTOOL_SCHEMAS_RULE@

comicsdir = $(comics_dir)
pixmapsdir = $(pixmaps_dir)
uidir = $(ui_dir)
schemadir = $(GCONF_SCHEMA_FILE_DIR)

comics_DATA = comics.xml
ui_DATA = buoh-ui.xml
pixmaps_DATA =     		\
             buoh64x64.png 	\
             buoh16x16.png

schema_in_files = buoh.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

Applicationsdir = $(datadir)/applications
Applications_in_files = buoh.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_DATA)

DISTCLEANFILES =	 	\
		buoh.desktop    \
		*.bak		\
		*.gladep	\
		$(schema_DATA)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                    	\
		$(comics_DATA)		\
		$(ui_DATA)		\
		$(pixmaps_DATA)		\
		$(schema_in_files)      \
		$(Applications_in_files)


