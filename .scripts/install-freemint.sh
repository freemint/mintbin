#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

# no need to install anything here.
# we just need the hosts compiler
exit 0

DOWNLOAD_DIR=http://tho-otto.de/snapshots
CROSS_TOOL=${CROSS_TOOL:-m68k-atari-mint}
SYSROOT_DIR=${SYSROOT_DIR:-"/usr/$CROSS_TOOL/sys-root/usr"}

sudo mkdir -p "${SYSROOT_DIR}" && cd "${SYSROOT_DIR}"

for package in $*
do
	wget -q -O - "$DOWNLOAD_DIR/${package}/${package}-latest.tar.bz2" | sudo tar xjf -
done
