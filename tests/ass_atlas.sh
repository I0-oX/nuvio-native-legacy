#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# Atlas das legendas ASS (um desenho por quadro) e matriz YCbCr do script
# (src/assrender.c, caminho novo do .tpk). Ver tests/ass_atlas.c. Precisa do
# libass (pkg-config); sem ele, pula.
set -eu
cd "$(dirname "$0")/.."
if ! command -v pkg-config >/dev/null 2>&1 || ! pkg-config --exists libass; then
  echo "ass_atlas: libass ausente (teste ignorado)"; exit 0
fi
SDLI="${NV_SDL_INC:-$(pkg-config --cflags-only-I sdl2 2>/dev/null | sed 's#/SDL2##') -I/opt/homebrew/include}"
GL=""; [ "$(uname)" = Darwin ] && GL="-framework OpenGL"
cc -DNV_ASS_LIBASS -DNV_ASS_NOVO -DNV_ASSPERF_TESTE -DNV_ASS_ATLAS_TESTE -DNV_ASS_ATLAS=1 -include tests/ass_atlas_gl.h \
  -Isrc $SDLI tests/ass_atlas.c src/assrender.c src/assperf.c src/assfios.c tests/ass_render_stub.c \
  -o "$TMPDIR/nuvio-ass-atlas-test" \
  $(pkg-config --cflags --libs libass) -lSDL2 -lpthread -lm $GL -Wno-deprecated-declarations -Wno-macro-redefined
"$TMPDIR/nuvio-ass-atlas-test" 2>/dev/null
