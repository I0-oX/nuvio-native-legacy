#!/bin/bash
# Faixa ASS com ContentEncoding (header stripping / zlib / cifra). Ver tests/mkvass_codificada.c.
#   bash tests/mkvass_codificada.sh
set -eu
cd "$(dirname "$0")/.."
DIR=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-mkvass-codificada.XXXXXX")
trap 'rm -rf "$DIR"' EXIT
FONTES="tests/mkvass_codificada.c src/mkvass.c src/imgsub.c src/pgs.c src/legenda.c src/assrender.c src/dados.c"
cc -Isrc -O1 -g -Wall -Wextra -Wno-deprecated-declarations $FONTES -o "$DIR/test" -lz -lm
mkdir "$DIR/dados"
for modo in strip zlib cifra; do
  (cd "$DIR" && NUVIO_DADOS="$DIR/dados" ./test "$modo")
  rm -rf "$DIR/dados"/*
done
