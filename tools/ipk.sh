#!/bin/bash
# Nuvio como .ipk para LG webOS, em um comando (par do tools/tpk.sh).
#
#   bash tools/ipk.sh                # imagem SDK (se faltar) + .ipk em build/ipk/
#   bash tools/ipk.sh --high-cache   # variante de 300 MB de texturas (-highcache)
#
# Nao toca na TV: roda `tools/arm.sh --ipk --build`, que empacota de uma copia
# limpa (sem credenciais de pessoa). A imagem nuvio-webos-sdk (tools/Dockerfile)
# traz o libass com os patches NEON de tools/tpk/libass-nv; se ela for de antes
# desses patches, o arm.sh recusa e pede para reconstruir (FORCE_IMAGE=1 refaz).
#
# ares-package: NUVIO_ARES_PACKAGE, ou o que estiver no PATH.
set -e
cd "$(dirname "$0")/.." && source tools/ctr.sh

SDK_IMAGE="${NUVIO_SDK_IMAGE:-nuvio-webos-sdk}"
BUILD_PLATFORM="${NUVIO_BUILD_PLATFORM:-linux/arm64}"
export NUVIO_ARES_PACKAGE="${NUVIO_ARES_PACKAGE:-$(command -v ares-package || true)}"
[ -n "$NUVIO_ARES_PACKAGE" ] || { echo "ares-package ausente: instale o webOS CLI ou defina NUVIO_ARES_PACKAGE" >&2; exit 2; }

if [ "${FORCE_IMAGE:-0}" = 1 ] || ! ctr_image_exists "$SDK_IMAGE" 2>/dev/null; then
  echo "==> imagem $SDK_IMAGE (demora na primeira vez)"
  ctr_build --platform "$BUILD_PLATFORM" -t "$SDK_IMAGE" tools/
fi

mkdir -p build/ipk
NUVIO_RELEASE_OUT="$PWD/build/ipk" bash tools/arm.sh --ipk --build "$@"
ls -la build/ipk/*.ipk
