#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# gfx_marca_lote contra gfx_rect um a um, num GL de verdade (GLES2). Nao entra
# na suite (*_shot.sh): precisa de janela GL (no Linux sem tela: xvfb-run).
#   bash tests/gfx_lote_shot.sh
set -eu
cd "$(dirname "$0")/.."
sources=()
for source in src/*.c src/dts/*.c; do [ "$source" != src/main.c ] && sources+=("$source"); done
if [ "$(uname)" = Darwin ]; then
  PLAT=(-I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -framework OpenGL)
else
  PLAT=($(pkg-config --cflags sdl2) -lGLESv2 -ljpeg -lwebp -lwebpdemux -lssl -lcrypto -ldl)
fi
ASS=(); command -v pkg-config >/dev/null && pkg-config --exists libass && ASS=(-DNV_ASS_LIBASS $(pkg-config --cflags --libs libass fribidi))
cc "${sources[@]}" tests/gfx_lote_shot.c -Isrc "${ASS[@]}" -o "$TMPDIR/nuvio-gfx-lote-shot" -O1 -g \
  "${PLAT[@]}" -lSDL2 -lSDL2_image -lSDL2_ttf -lz -lpthread -lm \
  -Wno-deprecated-declarations -Wno-macro-redefined -w
D=$(mktemp -d); trap 'rm -rf "$D"' EXIT
NUVIO_DADOS="$D" NUVIO_TESTE_DIR="$D" "$TMPDIR/nuvio-gfx-lote-shot"
