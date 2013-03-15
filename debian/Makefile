VERSION=$(shell ./get_version.sh)
MYDIST=$(shell cat /etc/lsb-release | grep CODENAME | sed -e 's/[^=]*[=]//')
SUBVERSION=2
PACKAGE=libgtklistmodel
DEPENDS=libpcre3,elementals,gtk+-3.0
DEPENDS1=
BUILDDEPENDS=automake,autoconf,intltool,libpthread-stubs0-dev,libpcre3-dev,pkg-config,elementals,libgtk-3-dev
DISTROS=oneiric precise quantal
DISTROS1=maverick lucid
DVERSION=${VERSION}.${DISTRO}.${SUBVERSION}
UBUNTU=/tmp/libgtklistmodel_ppa
UDIR=${UBUNTU}/${PACKAGE}-${DVERSION}
DDIR=${UDIR}/debian
PPA=ppa:0k-hans-f8/ppa

all:
	@echo "make srctar - makes a source tar which needs automake/autoconf"
	@echo ""
	@echo "make ppa - makes a PPA for launchpad for Ubuntu's distros:"
	@echo ""
	@echo "       ${DISTROS}"
	@echo ""
	@echo "make ppa_old - makes a PPA for launchpad for Ubuntu's distros:"
	@echo ""
	@echo "       ${DISTROS1}"
	@echo ""

srctar: 
	rm -rf /tmp/${PACKAGE}-${VERSION} 
	if [ -r ${PACKAGE}/Makefile ];then (cd ${PACKAGE};make distclean); fi
	mkdir /tmp/${PACKAGE}-${VERSION}
	(cd ..;tar cf - . ) | (cd /tmp/${PACKAGE}-${VERSION}; tar xf -)
	(cd /tmp/${PACKAGE}-${VERSION};automake;autoconf)
	(cd /tmp;tar -c -z --exclude .git -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION})
	mv /tmp/${PACKAGE}-${VERSION}.tar.gz .
	rm -rf /tmp/${PACKAGE}-${VERSION} 

clean:
	rm -f *.tar.gz
	rm -rf ${UBUNTU}

test: clean
	make build DISTRO=${MYDIST} DEPEND=${DEPENDS}
	(cd ${UBUNTU};sudo pbuilder build *.dsc)

ppa: clean
	for distro in ${DISTROS}; do make clean;make ppa1 DISTRO=$$distro DEPEND=${DEPENDS}; done

ppa_old: clean
	for distro in ${DISTROS1}; do make ppa1 DISTRO=$$distro DEPEND=${DEPENDS1}; done

ppa1: build
	(cd ${UBUNTU};dput ${PPA} *.changes)

build: srctar
	echo Making PPA for ${DISTRO}
	rm -rf ${UBUNTU}
	mkdir ${UBUNTU}
	cp ${PACKAGE}-${VERSION}.tar.gz ${UBUNTU}/${PACKAGE}_${DVERSION}.orig.tar.gz
	(cd ${UBUNTU}; tar xzf ${PACKAGE}_${DVERSION}.orig.tar.gz)
	mv ${UBUNTU}/${PACKAGE}-${VERSION} ${UDIR}
	tar cf - debian | (cd ${UDIR}; tar xf -)
	cat ${DDIR}/control.in | sed -e 's/%%DEPENDS%%/${DEPEND}/g' | sed -e 's/%%BUILDDEPENDS%%/${BUILDDEPENDS}/g' >${DDIR}/control
	rm ${DDIR}/control.in
	cat ${DDIR}/changelog.in | sed -e 's/%%VERSION%%/${VERSION}/g' | sed -e 's/%%DISTRO%%/${DISTRO}/g' | sed -e 's/%%SUBVERSION%%/${SUBVERSION}/g' >${DDIR}/changelog
	rm ${DDIR}/changelog.in
	(cd ${UDIR};debuild -S)




