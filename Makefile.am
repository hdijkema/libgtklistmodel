## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

libgtklistmodeldocdir = ${prefix}/doc/libgtklistmodel
libgtklistmodeldoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(libgtklistmodeldoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(libgtklistmodeldocdir)
