comics_DATA = comics.xml
ui_DATA = buoh.glade
pixmaps_DATA =     		\
             buoh64x64.png 	\
             buoh16x16.png

DISTCLEANFILES =	 	\
		buoh.desktop    \
		*.bak		\
		*.gladep

Applicationsdir = $(datadir)/applications
Applications_in_files = buoh.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                    	\
		$(comics_DATA)		\
		$(ui_DATA)		\
		$(pixmaps_DATA)		\
		$(Applications_in_files)


