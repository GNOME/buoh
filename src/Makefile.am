INCLUDES = 						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	$(BUOH_CFLAGS)					\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED)

bin_PROGRAMS = buoh

buoh_SOURCES = 							\
	buoh-comic.h buoh-comic.c				\
	comic-simple.h comic-simple.c				\
	buoh-window.h buoh-window.c				\
	buoh-comic-loader.h buoh-comic-loader.c			\
	buoh-view.h buoh-view.c					\
	buoh-view-comic.h buoh-view-comic.c	                \
	buoh-view-message.h buoh-view-message.c			\
	buoh-comic-list.h buoh-comic-list.c			\
	buoh-properties-dialog.h buoh-properties-dialog.c	\
	buoh-add-comic-dialog.h buoh-add-comic-dialog.c		\
	buoh.h buoh.c						\
	main.c

buoh_LDADD = $(BUOH_LIBS)

#EXTRA_DIST  = buoh.desktop.in 

#DISTCLEANFILES = 		\
#		buoh.desktop	\
#		*.bak

#Applicationsdir = $(datadir)/applications
#Applications_in_files = buoh.desktop.in
#Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)
#@INTLTOOL_DESKTOP_RULE@

#install-data-local:
#	@$(NORMAL_INSTALL)
