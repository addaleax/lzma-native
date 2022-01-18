#!/bin/sh

echo
echo "--- liblzma-build.sh $*"
echo "--- CWD = $PWD"
echo

set -e

case $(uname | tr '[:upper:]' '[:lower:]') in
  *bsd) alias make='gmake';;
  *)
esac

set -x
cd "$1" && cd liblzma
make
make install
