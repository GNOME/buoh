SUBDIRS = \
		src		\
		po		\
		data

intltool_extra = intltool-extract.in intltool-merge.in intltool-update.in

EXTRA_DIST = \
	$(intltool_extra)	\
	autogen.sh			\
	MAINTAINERS

DISTCLEANFILES = \
	intltool-extract	\
	intltool-merge		\
	intltool-update

MAINTAINERCLEANFILES = \
		ChangeLog \
		ChangeLog.pre-0.8.3 \
		$(srcdir)/INSTALL \
		$(srcdir)/aclocal.m4 \
		$(srcdir)/autoscan.log \
		$(srcdir)/compile \
		$(srcdir)/config.guess \
		$(srcdir)/config.h.in \
		$(srcdir)/config.sub \
		$(srcdir)/configure \
		$(srcdir)/configure.scan \
		$(srcdir)/depcomp \
		$(srcdir)/install-sh \
		$(srcdir)/ltmain.sh \
		$(srcdir)/missing \
		$(srcdir)/mkinstalldirs \
		$(srcdir)/omf.make \
		$(srcdir)/xmldocs.make \
		$(srcdir)/gtk-doc.make \
		$(srcdir)/po/Makefile.in.in \
		`find "$(srcdir)" -type f -name Makefile.in -print`
