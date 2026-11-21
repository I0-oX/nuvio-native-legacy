#!/bin/bash
# TorBox burn-in (tbburn.h): playlist/JSON parsing, the createstream route,
# plan refusals, eligibility and the playback row. Stubs, no network.
#   bash tests/tbburn.sh     SANITIZE=1 (ASan/UBSan)
set -euo pipefail
cd "$(dirname "$0")/.."
D=$(mktemp -d "${TMPDIR:-/tmp}/nv-tbburn.XXXXXX"); trap 'rm -rf "$D"' EXIT
# src/tbburn.c includes ajustes.h (<SDL2/SDL.h>): the SDL2 headers' parent
# folder (Homebrew on the Mac; NV_SDL_INC points elsewhere), as tpkborda.sh.
SDLI="${NV_SDL_INC:-$(pkg-config --cflags-only-I sdl2 2>/dev/null | sed 's#/SDL2##') -I/opt/homebrew/include}"
flags=()
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all); fi
cc "${flags[@]}" -Isrc $SDLI -O1 -g -Wall -Wextra src/tbburn.c tests/tbburn.c -pthread -o "$D/t"
"$D/t"
cc -Isrc $SDLI -DNV_TPK -fsyntax-only -Wall -Wextra -Werror src/tbburn.c
echo "tbburn ok"
