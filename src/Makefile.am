SUBDIRS = 

DEFS = 	-DPLUGINS_DIR=\"$(plugins_dir)\" 	\
	-DDATA_DIR=\"$(datadir)\" 		\
	-I$(top_srcdir)/include 		\
	-I$(srcdir)/ -I$(TOP_DIR) 		\
	-I$(SRC_DIR)/include			\
	$(GTK2_CFLAGS) $(GLIB2_CFLAGS) $(CFLAGS)\
	$(LIBGLADE_CFLAGS) $(LIBDUPTOOLS_CFLAGS)    \
	$(LIBSSL_CFLAGS) $(GTHREADS2_CFLAGS)

AM_CPPFLAGS = -DPROGRAM_LOCALE_DIR=\"${PROGRAM_LOCALE_DIR}\"

bin_PROGRAMS = gtkcmphash
gtkcmphash_LDFLAGS = -export-dynamic -no-undefined 
gtkcmphash_LDADD = $(GLIB2_LIBS) $(GTK2_LIBS) $(LIBGLADE_LIBS)      \
	       	   $(LIBDUPTOOLS_LIBS) $(LIBSSL_LIBS) $(GTHREADS2_LIBS) $(BZ2_LIBS)

gtkcmphash_SOURCES = 			\
	gtkcmphash.c			\
	hashing_struct.c		\
	log_window.c			\
	file_io.c			\
	progress_window.c		\
	pref_window.c			\
	include/gtkcmphash.h		\
	include/hashing_struct.h	\
	include/log_window.h		\
	include/file_io.h		\
	include/progress_window.h	\
	include/pref_window.h		

gladefiles = gtkcmphash.glade
gladedir = $(datadir)/@PACKAGE@
glade_DATA = $(gladefiles)
EXTRA_DIST = $(gladefiles)