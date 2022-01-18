#!/bin/sh

echo
echo "--- liblzma-config.sh $*"
echo "--- CWD = $PWD"
echo

SRC_TARBALL="$2"
TARGET_DIR="$1/liblzma"
mkdir -p "$TARGET_DIR"

tar xjf "$SRC_TARBALL" -C "$TARGET_DIR" || exit 1

cd "$TARGET_DIR"
TARGET_DIR="$(pwd)" # ensure absolute since --prefix needs it

function autoconf() {
    ( cd xz-* ; sh ./autogen.sh --no-po4a )
    return $?
}

function configure() {
    sh xz-*/configure \
        --quiet --enable-silent-rules \
        --prefix="$TARGET_DIR/build" \
        CFLAGS="-fPIC $CFLAGS" \
        --enable-static \
        --disable-xz \
        --disable-xzdec \
        --disable-lzmadec \
        --disable-lzmainfo \
        --disable-lzma-links \
        --disable-rpath \
        --disable-shared \
        --disable-scripts
    return $?
}

set -x

# FIXME: Remove after XZ 5.3 is released
if [ $(uname) = "Darwin" -a $(uname -m) = "arm64" ]; then
    echo "--- Patching config.sub for Apple Silicon"
    sed -i \
        's/\tnone)/\tarm64-*)\n\t\tbasic_machine=$(echo $basic_machine | sed "s\/arm64\/aarch64\/")\n\t\t;;\n\t\tnone)/g' \
        xz-*/build-aux/config.sub
    echo
fi

configure # || exit 1 # uncomment to disable use of autoconf
if [ $? -ne 0 ] ; then
  echo
  echo "--- ./configure failed => trying to run autoconf first"
  echo "--- NOTE: This requires a full autoconf build environment, and so may also fail"
  echo
  autoconf && configure
fi
