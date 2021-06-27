#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

export ACLOCAL=aclocal
export AUTOMAKE=automake
CC=m68k-atari-mint-gcc RANLIB=m68k-atari-mint-ranlib ./configure --prefix=/usr --disable-nls --host=m68k-atari-mint --target=m68k-atari-mint && make

make DESTDIR="${INSTALL_DIR}" install
rm -f "${INSTALL_DIR}/usr/share/info/dir"
rm -rf "${INSTALL_DIR}/usr/include"

find "${INSTALL_DIR}" -type f -perm -a=x -exec m68k-atari-mint-strip -s {} \;
find "${INSTALL_DIR}" -type f \( -name '*.a' -o -name '*.o' \) -exec m68k-atari-mint-strip -S -X -w -N '.L[0-9]*' {} \;
