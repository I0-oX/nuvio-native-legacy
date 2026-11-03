// ASS (libass) SETTINGS FOR THE SAMSUNG .tpk AND THE LG webOS APP (owner,
// 23-26/10/2026; webOS 31/10).
//
// The frame-by-frame anime signs (the phone scene of Kaguya-sama, a "Signs &
// Songs" track with 36,944 blocks) made the subtitles late and jerky on the
// S95C. Each fix was tested on the TV behind its own option; the winners are
// now fixed here (the TPK column of assperf.c) and the test panel is gone.
//
// What the person can still change lives in Subtitles > Style on an ASS/SSA
// track (faixas.c, .tpk only): size and opacity of the dialogue, contrast and
// gamma to match the TV, render resolution and libass itself (on/off). They
// are kept in assperf.txt (data), on this TV only.
//
// THE .wgt AND ANDROID DO NOT CHANGE. assrender.c is shared: there ASS_NOVO is
// 0 and assperf_get returns 0, the behaviour from before 23/10. webOS has the
// engine (ASS_NOVO = 1) and the Subtitles > Style rows for ASS tracks (faixas.c);
// contrast and gamma are hidden there (calibrated on the Samsung picture).
// See ASS_NOVO_TPK below for what stays Samsung-only.
//
// BLUR: libass's own, on the CPU (NEON, 3 threads). A GPU blur was tried on
// the S95C (24-27/10) and removed: CPU only was faster and lighter there.
#ifndef NV_ASSPERF_H
#define NV_ASSPERF_H

#include <stddef.h>

/* THE ENGINE. Every .tpk (Tizen 4-9, armv7 NEON: the whole .tpk is built
 * with -mfpu=neon) uses the engine tuned on the S95C: libass's NEON bitmap
 * engine and 3 blur threads, Alpha 8-bit upload with a texture cache, upload
 * outside the queue lock, incremental batches with past events dropped after
 * 2 min (restored on a seek back), render 100 ms ahead with held frames,
 * render only on changes, authored ASS styles, the app's cue copy every 10 s,
 * subtitle blocks from the local relay with 6 MB parked ahead, attached fonts
 * up to 64 MB. ASS_NOVO = 0 on the .wgt and Android: the engine of before
 * 23/10. NV_ASS_NOVO turns all of it on in the computer tests.
 *
 * webOS (NV_WEBOS) has the engine too: the same libass (tools/build-ass-arm.sh),
 * the 8-bit upload with the texture cache, render ahead, render only on
 * changes, incremental batches with the pruning, render resolution Auto, the
 * dialogue opacity and the app's cue copy every 10 s. The Samsung-only parts
 * stay out, each for a reason that is not about the engine (ASS_NOVO_TPK):
 *   - authored ASS styles: they go with the "Preservado pelo ASS" rows of
 *     faixas.c (.tpk). webOS keeps the sheet of the person: what the person
 *     touched in the Style bar still wins over the file, as before;
 *   - relay blocks parked ahead: the relay is the .tpk's local mkvfix server
 *     (video_tpk.c); webOS plays the file itself, so no block ever arrives;
 *   - attached fonts: 64 MB fixed on the .tpk; on webOS 64 MB too, falling back
 *     to 1/8 of the free memory (16..64 MB) under 128 MB free. This is only an
 *     acceptance limit: it does not reserve memory and does not guard against OOM.
 * The frame grid, the picture sync, the GPU queue limit and the subtitle
 * edge (legimagem.c, telalag.c, tpkborda.c) are .tpk files and stay so. */
#if defined(NV_TPK) || defined(NV_WEBOS) || defined(NV_ASS_NOVO)
#define ASS_NOVO 1
#else
#define ASS_NOVO 0
#endif
#if defined(NV_TPK) || defined(NV_ASS_NOVO)
#define ASS_NOVO_TPK 1
#else
#define ASS_NOVO_TPK 0
#endif

// The person's options (Subtitles > Style on an ASS/SSA track, .tpk):
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

// Attached fonts cap (bytes). 64 MB on the .tpk; on webOS 64 MB, or the 16 MB
// floor while free memory is under 128 MB. It is inline so the host tests that
// compile mkvass.c need no assperf.c. Acceptance limit only, not an OOM guard.
// 16 MB elsewhere.
#if defined(NV_WEBOS)
#include <unistd.h>
static inline size_t assperf_fontes_teto_webos(void) {
  long pag = sysconf(_SC_AVPHYS_PAGES), tam = sysconf(_SC_PAGESIZE);
  unsigned long long livre = pag > 0 && tam > 0 ? (unsigned long long)pag * (unsigned long long)tam : 0;
  return (size_t)(livre >= (128ull << 20) ? 64ull << 20 : 16ull << 20);
}
#define ASSPERF_FONTES_TETO assperf_fontes_teto_webos()
#elif ASS_NOVO_TPK
#define ASSPERF_FONTES_TETO ((size_t)64 * 1024u * 1024u)
#else
#define ASSPERF_FONTES_TETO ((size_t)16 * 1024u * 1024u)
#endif
#define ASSPERF_ADIANTE_MS  (ASS_NOVO ? 100 : 0)        // render ahead
#define ASSPERF_PODA_MS     (ASS_NOVO ? 120000L : 0L)   // drop events that ended this long ago
#define ASSPERF_PARQUE      (ASS_NOVO_TPK ? (6L << 20) : 0L)   // relay blocks parked ahead, bytes
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
