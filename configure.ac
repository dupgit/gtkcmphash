AC_PREREQ(2.59)

AC_INIT([gtkcmphash], [0.0.3],
        [olivierdelhomme@gmail.com],
        [gtkcmphash])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([$PACKAGE_TARNAME], [$PACKAGE_VERSION])
#AC_LIBTOOL_WIN32_DLL

dnl directory requirement
TOP_DIR=`pwd`
SRC_DIR="${TOP_DIR}/src"
AC_SUBST(TOP_DIR)
AC_SUBST(SRC_DIR)

dnl Libraries requirements
GLIB2_VERSION=2.8.0
GTK2_VERSION=2.8.0
GTHREADS_VERSION=2.10.0
LIBGLADE_VERSION=1.110.0
LIBDUPTOOLS_VERSION=0.0.4
LIBSSL_VERSION=0.9.8

AC_SUBST(GLIB2_VERSION)
AC_SUBST(GTK2_VERSION)
AC_SUBST(GTHREADS_VERSION)
AC_SUBST(LIBGLADE_VERSION)
AC_SUBST(LIBDUPTOOLS_VERSION)
AC_SUBST(LIBSSL_VERSION)

AM_MAINTAINER_MODE

dnl Put the preprocessor macros in a header instead of command line 
AC_CONFIG_SRCDIR([src/gtkcmphash.c])

dnl Checks for headers
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday select socket strdup strerror strstr)

dnl dynamic libraries/plugins
# AC_LIBTOOL_DLOPEN
# AC_LIBLTDL_INSTALLABLE
# AC_SUBST(LTDLINCL)
# AC_SUBST(LIBLTDL)
# AC_DISABLE_STATIC
# AC_PROG_LIBTOOL
# AC_LIB_LTDL

dnl Checks for programs
AM_PROG_LIBTOOL

if test "x$prefix" = "xNONE"; then
	prefix="/usr/local"
fi

if test "x$localstatedir" = "x\${prefix}/var"; then
	localstatedir="$prefix/var"
fi

if test "x$sysconfdir" = "x\${prefix}/etc"; then
    sysconfdir="$prefix/etc"
fi

AC_SUBST(prefix)
AC_SUBST(localstatedir)
AC_SUBST(sysconfdir)


dnl **************************************************
dnl * Check for Gtkdoc.                              *
dnl **************************************************
AC_CHECK_PROG(GTKDOC, gtkdoc-mkdb, true, false)
AM_CONDITIONAL(HAVE_GTK_DOC, test -n $GTKDOC)
AC_SUBST(HAVE_GTK_DOC)

AC_ARG_ENABLE(gtk-doc, [  --enable-gtk-doc  Use gtk-doc to build documentation [default=no]], enable_gtk_doc="$enableval", enable_gtk_doc=no)
AM_CONDITIONAL(ENABLE_GTK_DOC, test x$enable_gtk_doc = xyes)

dnl ********************************************************
dnl languages
dnl ********************************************************
ALL_LINGUAS="en_GB"

dnl ********************************************************
dnl triggers gettext
dnl ********************************************************
AC_PROG_INTLTOOL([0.23])
GETTEXT_PACKAGE=gtkcmphash
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,["$GETTEXT_PACKAGE"],[gettext domain])
AM_GLIB_GNU_GETTEXT

dnl *******************************
dnl checking for glib
dnl *******************************
PKG_CHECK_MODULES(GLIB2,[glib-2.0 >= $GLIB2_VERSION])

dnl *******************************
dnl checking for gtk+
dnl *******************************
PKG_CHECK_MODULES(GKT2,[gtk+-2.0 >= $GTK2_VERSION])

dnl *******************************
dnl checking for gthreads
dnl *******************************
PKG_CHECK_MODULES(GTHREADS2,[gthread-2.0 >= $GTHREADS_VERSION])

dnl *******************************
dnl checking for libglade
dnl *******************************
PKG_CHECK_MODULES(LIBGLADE, [libglade-2.0 >= $LIBGLADE_VERSION])

dnl *******************************
dnl checking for liblats
dnl *******************************
PKG_CHECK_MODULES(LIBDUPTOOLS, [libduptools >= $LIBDUPTOOLS_VERSION])

dnl *******************************
dnl checking for ssl
dnl *******************************
PKG_CHECK_MODULES(LIBSSL, [libssl >= $LIBSSL_VERSION])

dnl *******************************
dnl configure for libbz2
dnl *******************************
BZ2_LIBS="-lbz2"
AC_SUBST(BZ2_LIBS)

AC_PROG_INSTALL

CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-declarations \
-Wbad-function-cast -Wcast-qual -Wcast-align -Wnested-externs -Wunused \
-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"

case $host in
    *linux*)
	AC_DEFINE_UNQUOTED(SYS_LINUX,1,[This is a Linux system])
	PROGRAM_LOCALE_DIR=[${datadir}/locale]
        AC_SUBST(PROGRAM_LOCALE_DIR)
	;;
    *mingw*)
	AC_DEFINE_UNQUOTED(SYS_MINGW,1,[This is a MinGW system])
     	AC_DEFINE_UNQUOTED(WINDOWS,1,[This is a Windows system])
     	AC_DEFINE_UNQUOTED(_WIN32,1,[This is a Windows system])
        CFLAGS="$CFLAGS -mms-bitfields -mwindows -mno-cygwin"
	LDFLAGS="$LDFLAGS"
        ;;
esac


dnl *******************************
dnl debug option
dnl *******************************
AC_ARG_ENABLE([debug],
     [  --enable-debug    Turn on debugging],
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
     esac],[debug=false])
if test x$debug = xtrue
then
 AC_DEFINE_UNQUOTED(PROGRAM_DEBUG,TRUE,[Debug mode On])
else
 AC_DEFINE_UNQUOTED(PROGRAM_DEBUG,FALSE,[Debug mode Off])
fi

dnl *******************************
dnl gcov option
dnl *******************************
AC_ARG_ENABLE([gcov],
     [  --enable-gcov     Enables gcov coverage tests],
     [case "${enableval}" in
       yes) gcov=true ;;
       no)  gcov=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gcov]) ;;
     esac],[gcov=false])
if test x$gcov = xtrue
then
 CFLAGS="-fprofile-arcs -ftest-coverage $CFLAGS "
else
 CFLAGS="$CFLAGS"
fi

dnl *******************************
dnl gprof option
dnl *******************************
AC_ARG_ENABLE([gprof],
     [  --enable-gprof    Enables gprof profiling],
     [case "${enableval}" in
       yes) gprof=true ;;
       no)  gprof=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gprof]) ;;
     esac],[gprof=false])
if test x$gprof = xtrue
then
 CFLAGS="-pg $CFLAGS"
else
 CFLAGS="$CFLAGS"
fi


AC_SUBST(LDFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(LIBGLADE_CFLAGS)
AC_SUBST(LIBGLADE_LIBS)
AC_SUBST(GLIB2_CFLAGS)
AC_SUBST(GLIB2_LIBS)
AC_SUBST(GTK2_CFLAGS)
AC_SUBST(GTK2_LIBS)
AC_SUBST(GTHREADS2_CFLAGS)
AC_SUBST(GTHREADS2_LIBS)
AC_SUBST(LIBDUPTOOLS_CFLAGS)
AC_SUBST(LIBDUPTOOLS_LIBS)
AC_SUBST(LIBSSL_CFLAGS)
AC_SUBST(LIBSSL_LIBS)

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile po/Makefile.in])
AC_OUTPUT

echo
echo "*** Dumping configuration ***"
echo 
echo "    - Host                   : $host"
echo "    - Compiler               : ${CC}"
echo "    - Generate documentation : $enable_gtk_doc"
echo "    - Prefix                 : $prefix"
echo "    - Exec prefix            : $exec_prefix"
echo "    - Data dir               : $datadir"
echo "    - locale dir             : ${PROGRAM_LOCALE_DIR}"
echo
echo "    You can now run 'make' to compile gtkcmphash"
echo
