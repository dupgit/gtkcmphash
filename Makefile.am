SUBDIRS = src doc po

INTLTOOL_FILES =            \
	intltool-extract.in \
	intltool-merge.in   \
	intltool-update.in

M4_FILES =              \
	m4/codeset.m4   \
	m4/gettext.m4   \
	m4/glibc21.m4   \
	m4/iconv.m4     \
	m4/isc-posix.m4 \
	m4/lcmessage.m4 \
	m4/progtest.m4

DISTCLEANFILE =                   \
	intltool-extract          \
	intltool-merge            \
	intltool-update           \
	po/.intltool-merge-cache

WINDOWS_INSTALL =       \
	gtkcmphash.iss

MISC_FILES =		\
	autoclean.sh 	\
	autogen.sh

EXTRA_DIST = $(INTLTOOL_FILES) $(M4_FILES) $(WINDOWS_INSTALL) $(MISC_FILES)
