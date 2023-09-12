#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

export ACLOCAL=aclocal
export AUTOMAKE=automake
CROSS_TOOL=${CROSS_TOOL:-m68k-atari-mint}
# wrong: we need the cross-compiler version here
# CC=${CROSS_TOOL}-gcc RANLIB=m68k-atari-mint-ranlib ./configure --prefix=/usr --disable-nls --host=${CROSS_TOOL} --target=${CROSS_TOOL} && make

CC=gcc ./configure --prefix=/usr --disable-nls --target=${CROSS_TOOL}

make DESTDIR="${INSTALL_DIR}" install
rm -f "${INSTALL_DIR}/usr/share/info/dir"
rm -rf "${INSTALL_DIR}/usr/include"

# find "${INSTALL_DIR}" -type f -perm -a=x -exec ${CROSS_TOOL}-strip -s {} \;
# find "${INSTALL_DIR}" -type f \( -name '*.a' -o -name '*.o' \) -exec ${CROSS_TOOL}-strip -S -X -w -N '.L[0-9]*' {} \;

strip ${INSTALL_DIR}/usr/bin/*
