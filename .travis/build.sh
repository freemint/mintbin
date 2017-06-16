#!/bin/sh

TMP="$1"
OUT="$2"

export ACLOCAL=aclocal
export AUTOMAKE=automake
CC=m68k-atari-mint-gcc RANLIB=m68k-atari-mint-ranlib ./configure --prefix=/ --disable-nls --host=m68k-atari-mint --target=m68k-atari-mint && make

make DESTDIR="${TMP}" install
rm -rf "${TMP}/m68k-atari-mint"
find "${TMP}" -type f -perm -a=x -exec m68k-atari-mint-strip -s {} \;
find "${TMP}" -type f \( -name '*.a' -o -name '*.o' \) -exec m68k-atari-mint-strip -S -x {} \;
mkdir -p "${OUT}" && cd "${TMP}" && tar cjf "${OUT}/${PROJECT}-${SHORT_ID}.tar.bz2" *
