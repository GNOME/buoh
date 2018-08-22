INCLUDES = 											\
	-DDATADIR=\"$(pkgdatadir)\"						\
	-DGNOMEDATADIR=\"$(datadir)\"					\
	-DLOCALE_DIR=\"$(datadir)/locale\"			\
	-DGNOMEICONDIR=\""$(datadir)/pixmaps"\"			\
	-DBINDIR=\""$(bindir)"\"						\
	$(BUOH_CFLAGS)									\
	$(WARN_CFLAGS)									\
	$(DISABLE_DEPRECATED)

bin_PROGRAMS = buoh

buoh_SOURCES = \
	buoh-comic.h 					\
	buoh-comic.c					\
	buoh-window.h					\
	buoh-window.c					\
	buoh-comic-loader.h				\
	buoh-comic-loader.c				\
	buoh-view.h						\
	buoh-view.c						\
	buoh-view-comic.h				\
	buoh-view-comic.c				\
	buoh-view-message.h				\
	buoh-view-message.c				\
	buoh-comic-list.h				\
	buoh-comic-list.c				\
	buoh-properties-dialog.h		\
	buoh-properties-dialog.c		\
	buoh-add-comic-dialog.h			\
	buoh-add-comic-dialog.c			\
	buoh-comic-manager-date.h		\
	buoh-comic-manager-date.c		\
	buoh-comic-manager.h			\
	buoh-comic-manager.c			\
	buoh-comic-cache.h				\
	buoh-comic-cache.c				\
	buoh.h							\
	buoh.c							\
	main.c

buoh_LDFLAGS = $(AM_LDFLAGS)

buoh_LDADD = $(BUOH_LIBS)
