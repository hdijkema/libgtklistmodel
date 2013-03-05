## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(LIBGTKLISTMODEL_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libgtklistmodel.la


libgtklistmodel_la_SOURCES = \
	libgtklistmodel.c

libgtklistmodel_la_LDFLAGS = 

libgtklistmodel_la_LIBADD = $(LIBGTKLISTMODEL_LIBS)

include_HEADERS = \
	gtklistmodel.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgtklistmodel.pc

EXTRA_DIST = \
	libgtklistmodel.pc.in
