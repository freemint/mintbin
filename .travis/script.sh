#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

./configure --target=m68k-atari-mint --prefix=/usr --disable-nls
make
make install-strip DESTDIR="${INSTALL_DIR}"
