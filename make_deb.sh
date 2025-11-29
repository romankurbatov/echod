#!/bin/sh

PACKAGE_NAME=echod
VERSION=1.0
PACKAGE_REVISION=1

PACKAGE_DIR=${PACKAGE_NAME}_${VERSION}-${PACKAGE_REVISION}

DESTDIR=${PACKAGE_DIR}/usr/local make install

mkdir -p ${PACKAGE_DIR}/DEBIAN
cat >${PACKAGE_DIR}/DEBIAN/control <<EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}-${PACKAGE_REVISION}
Architecture: amd64
Maintainer: Roman Kurbatov <roman.kurbatov.1992@gmail.com>
Description: echo server with several commands
EOF

fakeroot <<EOF
chown -R root:root ${PACKAGE_DIR}
dpkg-deb --build ${PACKAGE_DIR}
EOF
