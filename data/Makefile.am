comicsdir = $(comics_dir)
pixmapsdir = $(pixmaps_dir)
uidir = $(ui_dir)

comics_DATA = comics.xml
ui_DATA = buoh-ui.xml
pixmaps_DATA =     		\
             buoh64x64.png 	\
             buoh16x16.png

Applicationsdir = $(datadir)/applications
Applications_in_files = buoh.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)

DISTCLEANFILES =	 	\
		buoh.desktop    \
		*.bak		\
		*.gladep
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                    	\
		$(comics_DATA)		\
		$(ui_DATA)		\
		$(pixmaps_DATA)		\
		$(Applications_in_files)


