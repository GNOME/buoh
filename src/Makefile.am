INCLUDES = 							\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"                \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED         \
	-DGNOME_DISABLE_DEPRECATED      \
	@BUOH_CFLAGS@

LDADD =				\
	$(BUOH_LIBS)	

bin_PROGRAMS = buoh

buoh_SOURCES = \
	callbacks.c callbacks.h\
	comic.h comic.c\
	comic-simple.h comic-simple.c\
	buoh.h buoh.c\
	main.c

buoh_LDADD = @BUOH_LIBS@

EXTRA_DIST  = \
	buoh.desktop.in 

DISTCLEANFILES = \
		buoh.desktop	\
		*.bak

Applicationsdir = $(datadir)/applications
Applications_in_files = buoh.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	@$(NORMAL_INSTALL)
