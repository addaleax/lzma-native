#!/bin/sh
set -e

SRC_TARBALL="$2"
TARGET_DIR="$1/liblzma"

mkdir -p "$TARGET_DIR"
cd "$TARGET_DIR"

tar xvjf "$SRC_TARBALL" >node_liblzma_config.log 2>&1

export CFLAGS="-fPIC $CFLAGS"

XZ_SRC_DIR=$(ls | grep xz-*)

# Fix build on Apple Silicon
if [ $(uname) = "Darwin" -a $(uname -m) = "arm64" ]; then
    sed -i '' 's/\tnone)/\tarm64-*)\n\t\tbasic_machine=$(echo $basic_machine | sed "s\/arm64\/aarch64\/")\n\t\t;;\n\t\tnone)/g' $XZ_SRC_DIR/build-aux/config.sub
fi

if [ $(uname) = "Linux" ]; then
    find $XZ_SRC_DIR -name config.guess -exec cp -f /usr/share/libtool/build-aux/config.guess {} \;
fi

sh xz-*/configure --enable-static --disable-shared --disable-scripts --disable-lzmainfo \
    --disable-lzma-links --disable-lzmadec --disable-xzdec --disable-xz --disable-rpath \
    --prefix="$TARGET_DIR/build" CFLAGS="$CFLAGS" >>node_liblzma_config.log 2>&1
