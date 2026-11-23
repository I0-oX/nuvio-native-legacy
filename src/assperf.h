// ASS (libass) SETTINGS FOR THE SAMSUNG .tpk AND THE LG webOS APP (owner,
// 23-26/10/2026; webOS 31/10).
//
// The frame-by-frame anime signs (the phone scene of Kaguya-sama, a "Signs &
// Songs" track with 36,944 blocks) made the subtitles late and jerky on the
// S95C. Each fix was tested on the TV behind its own option; the winners are
// now fixed here (the TPK column of assperf.c) and the test panel is gone.
//
// What the person can still change lives in Subtitles > Style on an ASS/SSA
// track (faixas.c, .tpk and webOS): size and opacity of the dialogue, contrast
// and gamma to match the TV (Samsung only), render resolution, picture sync
// (.tpk and webOS; the S95C's values as the starting point on webOS) and
// libass itself (on/off).
// They are kept in assperf.txt (data), on this TV only.
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
 * dialogue opacity, the authored ASS styles ("Preservado pelo ASS") and the
 * app's cue copy every 10 s. The Samsung-only parts
 * stay out, each for a reason that is not about the engine (ASS_NOVO_TPK):
 *   - relay blocks parked ahead: the relay is the .tpk's local mkvfix server
 *     (video_tpk.c); webOS plays the file itself, so no block ever arrives;
 *   - attached fonts: 64 MB fixed on the .tpk; on webOS 64 MB too, falling back
 *     to 1/8 of the free memory (16..64 MB) under 128 MB free. This is only an
 *     acceptance limit: it does not reserve memory and does not guard against OOM.
 * The GPU queue limit (telalag.c) runs on webOS too: ES 3 core fences when the
 * context has them, else EGL_KHR_fence_sync, else nothing (one log line). The
 * frame grid and the subtitle edge (tpkborda.c) are .tpk files and stay so;
 * the picture sync (legimagem.c) runs on both, the kind of video coming from
 * the file's own header (video_tpk_quadro: the Samsung host's MKV header on
 * the .tpk, video_quadro_fps from the MKV probe on webOS). */
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

// The person's options (Subtitles > Style on an ASS/SSA track, .tpk and webOS):
enum {
  AP_ESCALA,       // render resolution (AP_RES_*): Low / Medium / High auto (default by Tizen version)
  AP_GAMA,         // gamma like the TV's slider, signed: -20..+20 (0 off; the TV stops at 5)
  AP_CONTRASTE,    // contrast as the TV's number minus 30: -30..+20 (0 = TV default 30, gain 1.0)
  AP_LIBASS,       // 1 = libass draws ASS/SSA; 0 = dialogue only, as plain text
  AP_ASSTAM,       // dialogue size, %: 50..200 (100)
  AP_ASSOPAC,      // dialogue opacity, %: 20..100 (100)
  AP_N
};
/* The renderer's test rows are gone (owner 23/11/2026, "we have good values"):
 * split frames Per image, merged positions Exact, authored ASS hinting None
 * (plain text keeps Light), large caches, scripts of 540 lines or fewer at
 * 100 % in the Autos, busy scenes render only the newest frame. Custom auto
 * and its text / blur rows went with them; a saved Custom (6) or any old key
 * loads as the TV's default and is dropped on the next save. KeijoMika's
 * always-on savings (event index, signature cache, hashing) stay. */
/* The TV's Tizen major version (video_tpk.c), for the default render mode. */
void assperf_tizen_definir(int maior);

/* AP_ESCALA values (owner 22/11/2026): three Autos, all at
 * 100 % in quiet scenes; while busy the text and the blur are drawn at their
 * own resolutions (libass split passes, 0004).
 *   Low auto (was "Auto 50 %" = 5):  text 50 %, blur clean near 40 % (one pass at 50 when 50 is clean).
 *   Medium auto (was "Auto 75 %" = 3): text 75 %, blur clean near 50 %.
 *   High auto (7):                      text 100 %, blur clean near 50 % (owner 23/11/2026, LOG 8:
 *                                       text 100 costs the same as 90 on the S95C).
 * Default mode by the TV (owner 23/11/2026; a saved choice wins): Tizen 7+
 * High, Tizen 6/6.5 Medium, Tizen 4/5 (and the NuvioTpk40 library) Low;
 * version unknown: Medium (assperf_tizen_definir).
 * Saved fixed 100 % (0) reads as High auto, fixed 75 % (2) as Medium auto;
 * a saved Custom auto (6, removed 23/11/2026) as the TV's default.
 * "Clean" blur (no horizontal lines on gradients built from thin strips):
 * screen pixels per script pixel f = frame height (layout x k x %) / PlayResY
 * is one of 1/4, 1/3, 2/5, 1/2, 1, 2, 3, 4. The Auto takes the lowest clean %
 * at or above its target (25..75 %), else the highest clean % up to 75, else
 * the target. Scripts of 540 lines or fewer: text 100 %, blur the highest
 * clean % up to 100 (owner: 480p renders at 100). */
#define AP_RES_100_VELHO 0   // only read from old files: High auto
#define AP_RES_75_VELHO  2   // only read from old files: Medium auto
#define AP_RES_MEDIO     3
#define AP_RES_BAIXO     5
#define AP_RES_CUSTOM_VELHO 6   // only read from old files: the TV's default
#define AP_RES_ALTO      7
#define AP_RES_AUTO(v) ((v) == AP_RES_BAIXO || (v) == AP_RES_MEDIO || (v) == AP_RES_ALTO)
#define AP_BAIXO_TEXTO 0.50
#define AP_BAIXO_BLUR  0.40
#define AP_MEDIO_TEXTO 0.75
#define AP_MEDIO_BLUR  0.50
#define AP_ALTO_TEXTO  1.00
#define AP_ALTO_BLUR   0.50
#define AP_BLUR_TETO   0.75

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
#define ASSPERF_FIOS        (ASS_NOVO ? 3 : 1)          // NEON blur threads (Tizen 4/5: 2, assrender.c)
#define ASSPERF_ADIANTE_MS  (ASS_NOVO ? 100 : 0)        // render ahead
#define ASSPERF_PODA_MS     (ASS_NOVO ? 120000L : 0L)   // drop events that ended this long ago
#define ASSPERF_PARQUE      (ASS_NOVO ? (6L << 20) : 0L)   // relay blocks parked ahead, bytes
double assperf_escala(void);                 // render resolution now (Auto, busy: the text's)
double assperf_escala_blur(void);            // Auto, busy: the blur's; 0 = one pass
// The pixel grid the clean values are worked out on (worker): the text pass's
// frame height at 100 % (layout height x k) and the script's PlayResY.
void   assperf_grade_definir(int alturaPx, int playResY);
void   assperf_grade(int *alturaPx, int *playResY);
// The current mode's name and busy scales, for the log.
void   assperf_modo_texto(char *b, int n);
// Subtitle colour like the video's (AP_GAMA, AP_CONTRASTE): the TV's picture
// processing only touches the video plane.
double assperf_gama(void);                   // exponent: 1.0 = off
double assperf_contraste(void);              // gain: 0.70 (0) .. 1.0 (30) .. 1.20 (50)

// Render resolution Auto and the busy-scene flag (picture sync in legimagem.c).
// Worker, the next render: *texto = the scale of the (sharp) pass; returns 1
// with *blur = the soft pass's scale when this frame is split (an Auto mode,
// busy), else 0 and *blur = 0.
int    assperf_resolucao_quadro(double *texto, double *blur);
int    assperf_reduzido(void);               // an Auto mode is below 100 % now (telalag.c)
void   assperf_escala_reset(void);           // track change: back to 100 %
void   assperf_escala_custo(double libassMs);   // worker: a render that changed the picture
void   assperf_carga_gl(double ms);          // GL thread: cost of a new frame (0 = playing tick)
// Worker: lines starting in the next second, their text size and the pictures
// in this frame (busy-scene forecast); and the same for the second starting
// 0.5 s ahead (render resolution Auto only, so it drops to 75 % just before).
void   assperf_previsao(int inicios, long chars, int imagens, int iniciosCedo, long charsCedo);
int    assperf_pesado(void);                 // 1 = busy scene now
// Tizen 4-6 TVs (owner, 17/11/2026). Set by video_tpk.c from the Tizen
// version; always 1 in the Tizen 4/5 library; 0 elsewhere. Since 19/11/2026
// only the log reads it: busy scenes render further ahead on every .tpk, and
// the busy resolution is the person's choice (Auto 50 % is the default).
void   assperf_tv_lenta_definir(int sim);
int    assperf_tv_lenta(void);

#ifdef NV_ASSPERF_TESTE
void assperf_teste_definir(int id, int v);   // tests on the computer
int  assperf_teste_lido(int id, int v);       // a saved value as read (-1 = refused)
#endif

#endif
