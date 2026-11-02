// ASS (libass) SETTINGS FOR THE SAMSUNG .tpk AND THE LG webOS APP (owner,
// 23-26/10/2026; webOS 31/10).
//
// The frame-by-frame anime signs (the phone scene of Kaguya-sama, a "Signs &
// Songs" track with 36,944 blocks) made the subtitles late and jerky on the
// S95C. Each fix was tested on the TV behind its own option; the winners are
// now fixed here (the TPK column of assperf.c) and the test panel is gone.
//
// What the person can still change lives in Subtitles > Style on an ASS/SSA
// track (faixas.c, ASS_OPCOES: .tpk and webOS): size and opacity of the
// dialogue, contrast and gamma to match the TV, render resolution and libass
// itself (on/off). They are kept in assperf.txt (data), on this TV only.
//
// THE .wgt AND ANDROID DO NOT CHANGE. assrender.c is shared: there ASS_NOVO is
// 0 and assperf_get returns 0, the behaviour from before 23/10.
//
// BLUR: libass's own, on the CPU (NEON, cores - 1 threads). A GPU blur was
// tried on the S95C (24-27/10) and removed: CPU only was faster and lighter
// there.
#ifndef NV_ASSPERF_H
#define NV_ASSPERF_H

#include <stddef.h>

/* THE ENGINE AND WHO RUNS IT (the decision lives in assplat.h, one place):
 *
 * ASS_NOVO: libass's NEON bitmap engine and blur threads, Alpha 8-bit upload
 * with a texture cache, upload outside the queue lock, incremental batches
 * with past events dropped after 2 min (restored on a seek back), render
 * 100 ms ahead with held frames, render only on changes, the frame grid, the
 * dialogue-only font scale and opacity, the app's cue copy every 10 s, attached
 * fonts up to 16..64 MB. It runs on every .tpk (Tizen 4-9, armv7 NEON: the
 * whole .tpk is built with -mfpu=neon) and on webOS (the same libass,
 * tools/build-ass-arm.sh). ASS_NOVO = 0 on the .wgt and Android: the engine of
 * before 23/10. NV_ASS_NOVO turns it on in the computer tests, with the
 * Samsung numbers.
 *
 * ASS_OPCOES: the person's options and the Style rows, .tpk and webOS.
 *
 * ASS_NOVO_TPK is what depends on the Samsung host and nothing else:
 *   - authored ASS styles, with the "Preservado pelo ASS" rows of faixas.c:
 *     on webOS what the person touched in the Style bar still wins over the
 *     file, as before the engine;
 *   - relay blocks parked ahead: the relay is the .tpk's local mkvfix server
 *     (video_tpk.c); webOS plays the file itself, so no block ever arrives;
 *   - the picture sync (legimagem.c).
 * The picture sync, the GPU queue limit and the subtitle edge (legimagem.c,
 * telalag.c, tpkborda.c) are .tpk files and stay so.
 *
 * THE NUMBERS are not here: they are the ASSPERF_* names of assplat.h, one
 * block for the S95C (.tpk), one for webOS and one for the old engine. */
#include "assplat.h"

// The person's options (Subtitles > Style on an ASS/SSA track, .tpk and webOS):
enum {
  AP_ESCALA,       // render resolution: 0 100%, 2 75%, 5 Auto (100%, 75% while busy)
  AP_GAMA,         // gamma like the TV's slider: 0 off, 1..5 = -1..-5, 6..10 = +5..+1
  AP_CONTRASTE,    // contrast like the TV's: 0 = 30 (TV default) .. 30 = 0
  AP_LIBASS,       // 1 = libass draws ASS/SSA; 0 = dialogue only, as plain text
  AP_ASSTAM,       // dialogue size, %: 50..200 (100)
  AP_ASSOPAC,      // dialogue opacity, %: 20..100 (100)
  AP_N
};

// Current value (cheap): the person's option or its default (assperf.c). On
// the .wgt and Android: always 0.
int  assperf_get(int id);
// Reads assperf.txt once (later calls do nothing).
void assperf_iniciar(void);
// Saves one of the person's options.
void assperf_definir(int id, int v);
// The person's options back to their defaults.
void assperf_restaurar(void);
// Changes with every assperf_definir (the worker re-applies).
unsigned assperf_versao(void);

// Fixed values of the engine, named in assplat.h per platform (0 / the old
// value where ASS_NOVO is 0): ASSPERF_ADIANTE_MS (render ahead),
// ASSPERF_PODA_MS (drop events that ended this long ago), ASSPERF_PARQUE (relay
// blocks parked ahead, bytes). The attached fonts cap is a function: where
// ASSPERF_FONTES_DINAMICO is 1 (.tpk and webOS) 1/ASSPERF_FONTES_LIVRE_DIV of
// the memory free when the file is read, clamped to ASSPERF_FONTES_MIN..MAX
// (16..64 MB); the fixed 16 MB elsewhere.
size_t assperf_fontes_teto(void);
#define ASSPERF_FONTES_TETO assperf_fontes_teto()   // attached fonts, bytes
double assperf_escala(void);                 // render resolution now (Auto: the one in use)
// Subtitle colour like the video's (AP_GAMA, AP_CONTRASTE): the TV's picture
// processing only touches the video plane.
double assperf_gama(void);                   // exponent: 1.0 = off
double assperf_contraste(void);              // gain: 1.0 (30) .. 0.70 (0)

// Render resolution Auto and the busy-scene flag (picture sync in legimagem.c).
double assperf_escala_quadro(void);          // worker: the scale for the next render
void   assperf_escala_reset(void);           // track change: back to 100 %
void   assperf_escala_custo(double libassMs);   // worker: a render that changed the picture
void   assperf_carga_gl(double ms);          // GL thread: cost of a new frame (0 = playing tick)
// Worker: lines starting in the next second, their text size and the pictures
// in this frame (busy-scene forecast); and the same for the second starting
// 0.5 s ahead (render resolution Auto only, so it drops to 75 % just before).
void   assperf_previsao(int inicios, long chars, int imagens, int iniciosCedo, long charsCedo);
int    assperf_pesado(void);                 // 1 = busy scene now

#ifdef NV_ASSPERF_TESTE
void assperf_teste_definir(int id, int v);   // tests on the computer
#endif

#endif
