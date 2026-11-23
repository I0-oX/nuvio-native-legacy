// ASS (libass) SETTINGS FOR THE SAMSUNG .tpk (owner, 23-26/10/2026).
//
// The frame-by-frame anime signs (the phone scene of Kaguya-sama, a "Signs &
// Songs" track with 36,944 blocks) made the subtitles late and jerky on the
// S95C. Each fix was tested on the TV behind its own option; the winners are
// now fixed here (the TPK column of assperf.c) and the test panel is gone.
//
// What the person can still change lives in Subtitles > Style on an ASS/SSA
// track (faixas.c): size and opacity of the dialogue, contrast and gamma to
// match the TV, render resolution and libass itself (on/off). They are kept in
// assperf.txt (data), on this TV only.
//
// ONLY THE .tpk CHANGES. assrender.c is shared with webOS, the .wgt and
// Android: there ASS_NOVO is 0 and assperf_get returns 0, the behaviour from
// before 23/10.
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
 * up to 64 MB. ASS_NOVO = 0 everywhere else: the engine of before 23/10.
 * NV_ASS_NOVO turns it on in the computer tests. */
#if defined(NV_TPK) || defined(NV_ASS_NOVO)
#define ASS_NOVO 1
#else
#define ASS_NOVO 0
#endif

// The person's options (Subtitles > Style on an ASS/SSA track, .tpk):
enum {
  AP_ESCALA,       // render resolution (AP_RES_*): Low / Medium / High auto (default by Tizen version), Custom auto
  AP_GAMA,         // gamma like the TV's slider, signed: -20..+20 (0 off; the TV stops at 5)
  AP_CONTRASTE,    // contrast as the TV's number minus 30: -30..+20 (0 = TV default 30, gain 1.0)
  AP_LIBASS,       // 1 = libass draws ASS/SSA; 0 = dialogue only, as plain text
  AP_ASSTAM,       // dialogue size, %: 50..200 (100)
  AP_ASSOPAC,      // dialogue opacity, %: 20..100 (100)
  AP_TXTRES,       // Custom auto, busy: text resolution %, 25..100 in 5s (100)
  AP_BLURRES,      // Custom auto, busy: blur resolution % 25..100 in 5s, or AP_LIMPO_BASE+c = clean step c (50)
  /* Busy scenes (owner 22/11/2026): 0 Off = only the newest frame (signs
   * update less often, each on time); 1 Auto = every frame while the render
   * peak fits in 80 % of a video frame; 2 On = every frame always. (0) */
  AP_TODOS,
  /* KeijoMika's ass-minimal (owner 23/11/2026): each renderer difference of
   * ours as a row, so his build can be reproduced here. 0 = ours (default),
   * 1 = his, except AP_PEQ100 (1 = ours). The "Renderer preset" row of the
   * Style sheet sets them all at once (assperf_preset). */
  AP_DIVIDIDO,     // busy split frames: 1 per image through the texture cache (default since 23/11, LOG 3); 0 per-pass layers
  AP_POSINT,       // merged frames: 0 sub-pixel offsets exact (resampled); 1 whole pixel
  AP_HINT,         // authored ASS hinting: 0 none; 1 light
  AP_CACHEP,       // caches: 0 large (22/11); 1 small (libass 32/24 MB, rigid 384 / 32 MB, textures 768 / 24 MB, queue 24 MB)
  AP_PEQ100,       // scripts of 540 lines or fewer at 100 % text in the Autos: 1 on; 0 off
  AP_N
};
/* The Renderer preset (owner 23/11/2026): 0 Nuvio (ours), 1 KeijoMika (his),
 * 2 Custom (the rows mixed). assperf_preset_definir takes 0 or 1. */
int  assperf_preset(void);
/* The TV's Tizen major version (video_tpk.c), for the default render mode. */
void assperf_tizen_definir(int maior);
void assperf_preset_definir(int p);

/* AP_ESCALA values (owner 22/11/2026): three Autos and Custom auto, all at
 * 100 % in quiet scenes; while busy the text and the blur are drawn at their
 * own resolutions (libass split passes, 0004).
 *   Low auto (was "Auto 50 %" = 5):  text 50 %, blur clean near 40 % (one pass at 50 when 50 is clean).
 *   Medium auto (was "Auto 75 %" = 3): text 75 %, blur clean near 50 %.
 *   High auto (7):                      text 100 %, blur clean near 50 % (owner 23/11/2026, LOG 8:
 *                                       text 100 costs the same as 90 on the S95C).
 * Default mode by the TV (owner 23/11/2026; a saved choice wins): Tizen 7+
 * High, Tizen 6/6.5 Medium, Tizen 4/5 (and the NuvioTpk40 library) Low;
 * version unknown: Medium (assperf_tizen_definir).
 *   Custom auto (6): the Text / Blur resolution rows.
 * Saved fixed 100 % (0) reads as High auto, fixed 75 % (2) as Medium auto.
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
#define AP_RES_CUSTOM    6
#define AP_RES_ALTO      7
#define AP_RES_AUTO(v) ((v) == AP_RES_BAIXO || (v) == AP_RES_MEDIO || (v) == AP_RES_ALTO || (v) == AP_RES_CUSTOM)
#define AP_BAIXO_TEXTO 0.50
#define AP_BAIXO_BLUR  0.40
#define AP_MEDIO_TEXTO 0.75
#define AP_MEDIO_BLUR  0.50
#define AP_ALTO_TEXTO  1.00
#define AP_ALTO_BLUR   0.50
#define AP_BLUR_TETO   0.75
#define AP_LIMPO_BASE  1000  // AP_BLURRES = 1000 + c: the clean step c (1..AP_LIMPO_N) of the playing file
#define AP_LIMPO_N     8

// Current value (cheap). Outside the .tpk: always 0.
int  assperf_get(int id);
// Reads assperf.txt once (later calls do nothing).
void assperf_iniciar(void);
// Saves one of the person's options.
void assperf_definir(int id, int v);
// The person's options back to their defaults.
void assperf_restaurar(void);
// Changes with every assperf_definir (the worker re-applies).
unsigned assperf_versao(void);

// Fixed values of the engine (0 / the old value where ASS_NOVO is 0).
#define ASSPERF_FONTES_TETO ((size_t)(ASS_NOVO ? 64 : 16) * 1024u * 1024u)   // attached fonts, bytes
#define ASSPERF_FIOS        (ASS_NOVO ? 3 : 1)          // NEON blur threads
#define ASSPERF_ADIANTE_MS  (ASS_NOVO ? 100 : 0)        // render ahead
#define ASSPERF_PODA_MS     (ASS_NOVO ? 120000L : 0L)   // drop events that ended this long ago
#define ASSPERF_PARQUE      (ASS_NOVO ? (6L << 20) : 0L)   // relay blocks parked ahead, bytes
double assperf_escala(void);                 // render resolution now (Auto, busy: the text's)
double assperf_escala_blur(void);            // Auto, busy: the blur's; 0 = one pass
// The pixel grid the clean values are worked out on (worker): the text pass's
// frame height at 100 % (layout height x k) and the script's PlayResY.
void   assperf_grade_definir(int alturaPx, int playResY);
void   assperf_grade(int *alturaPx, int *playResY);
// Custom auto's Blur row: the clean values of the playing file within 25..100 %:
// percent x 10 and the AP_BLURRES code; returns how many (up to max).
int    assperf_blur_limpos(int *pct10, int *codigo, int max);
// AP_BLURRES as a scale (Custom auto), on the current grid.
double assperf_blurres_escala(int v);
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
/* Busy and still rendering only the newest frame (Every frame Off, or Auto
 * with a render peak `picoMs` over 80 % of a frame at `fps`). */
int    assperf_so_o_mais_novo(double picoMs, double fps);
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
