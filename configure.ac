AC_PREREQ(2.63.2)
AC_INIT([buoh],[0.8.3],[https://bugzilla.gnome.org/enter_bug.cgi?product=buoh],[buoh],[http://projects.gnome.org/buoh/])
AM_INIT_AUTOMAKE([1.10 dist-bzip2])

dnl Useful to call aclocal after 'make'
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS")

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2.6])
IT_PROG_INTLTOOL([0.40.0])

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_DIAGNOSE([obsolete],[AM_PROG_CC_STDC:
	your code should no longer depend upon `am_cv_prog_cc_stdc', but upon
	`ac_cv_prog_cc_stdc'.  Remove this warning and the assignment when
	you adjust the code.  You can also remove the above call to
	AC_PROG_CC if you already called it elsewhere.])
am_cv_prog_cc_stdc=$ac_cv_prog_cc_stdc

AC_HEADER_STDC
LT_INIT

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS
GNOME_MAINTAINER_MODE_DEFINES
GNOME_DEBUG_CHECK

AM_GLIB_GNU_GETTEXT

dnl ******************************************
dnl ** pkg-config dependacy checks          **
dnl ******************************************

GLIB_REQUIRED=2.26.0
GTK_REQUIRED=3.0.0
LIBSOUP_REQUIRED=2.4.0

PKG_CHECK_MODULES(BUOH, 
		  glib-2.0       >= $GLIB_REQUIRED
		  gtk+-3.0       >= $GTK_REQUIRED
		  libsoup-2.4    >= $LIBSOUP_REQUIRED)
AC_SUBST(BUOH_CFLAGS)
AC_SUBST(BUOH_LIBS)

GLIB_GSETTINGS

dnl
dnl Translations
dnl
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[The gettext package])
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

# Honor aclocal flags
  ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

  
if test "x${prefix}" = "xNONE"; then
   AC_DEFINE_UNQUOTED(UI_DIR, "${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/ui", [Path where glade files will be installed])
   ui_dir="${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/ui"
else
   AC_DEFINE_UNQUOTED(UI_DIR, "${prefix}/${DATADIRNAME}/${PACKAGE}/ui", [Path where glade files will be installed])
   ui_dir="${prefix}/${DATADIRNAME}/${PACKAGE}/ui"
fi

AC_SUBST(ui_dir)
			
if test "x${prefix}" = "xNONE"; then
   AC_DEFINE_UNQUOTED(COMICS_DIR, "${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/comics", [Path where data files will be installed])
   comics_dir="${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/comics"
else
   AC_DEFINE_UNQUOTED(COMICS_DIR, "${prefix}/${DATADIRNAME}/${PACKAGE}/comics", [Path where data files will be installed])
   comics_dir="${prefix}/${DATADIRNAME}/${PACKAGE}/comics"
fi

AC_SUBST(comics_dir)

AC_CONFIG_FILES([
Makefile
po/Makefile.in
src/Makefile
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/22x22/Makefile
data/icons/24x24/Makefile
data/icons/32x32/Makefile
data/icons/48x48/Makefile
data/icons/64x64/Makefile
])
AC_OUTPUT
