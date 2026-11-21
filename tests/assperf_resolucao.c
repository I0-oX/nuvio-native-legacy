// Render resolution modes (src/assperf.c, owner 19/11/2026): Auto 50 / Auto 75
// reduce the whole frame while busy; Custom auto splits text and blur.
#include "assperf.h"
#include <stdio.h>

static int falhas;
#define OK(c, m) do { if (c) printf("ok   %s\n", m); else { printf("FAIL %s\n", m); falhas++; } } while (0)

static int quadro(double *tx, double *bl) { return assperf_resolucao_quadro(tx, bl); }

int main(void) {
  double tx, bl; int d;
  assperf_teste_definir(AP_TXTRES, 100);
  assperf_teste_definir(AP_BLURRES, 50);
  assperf_teste_definir(AP_ESCALA, AP_RES_CUSTOM);
  d = quadro(&tx, &bl);
  OK(!d && tx == 1.0 && bl == 0.0, "custom auto, quiet scene: one pass at 100 %");
  OK(!assperf_reduzido() && assperf_escala_blur() == 0.0, "quiet: not reduced");
  assperf_previsao(400, 0, 50, 400, 0);   /* forecast: a busy scene */
  d = quadro(&tx, &bl);
  OK(d && tx == 1.0 && bl == 0.5, "custom auto, busy: text 100 %, blur 50 %");
  OK(assperf_reduzido() && assperf_escala_blur() == 0.5 && assperf_escala() == 1.0, "busy: reduced, scales logged");
  assperf_teste_definir(AP_TXTRES, 85); assperf_teste_definir(AP_BLURRES, 35);
  d = quadro(&tx, &bl);
  OK(d && tx == 0.85 && bl == 0.35, "the test rows apply at once (85 / 35)");
  assperf_teste_definir(AP_TXTRES, 60); assperf_teste_definir(AP_BLURRES, 60);
  d = quadro(&tx, &bl);
  OK(!d && tx == 0.6 && bl == 0.0, "text = blur: one pass at that scale");
  assperf_teste_definir(AP_TXTRES, 0); assperf_teste_definir(AP_BLURRES, 0);
  d = quadro(&tx, &bl);
  OK(d && tx == 1.0 && bl == 0.5, "custom auto: out-of-range rows fall back to 100 / 50");
  assperf_teste_definir(AP_ESCALA, AP_RES_AUTO75);
  d = quadro(&tx, &bl);
  OK(!d && tx == 0.75, "auto 75, busy: whole frame 75 %");
  assperf_teste_definir(AP_ESCALA, AP_RES_AUTO50);
  d = quadro(&tx, &bl);
  OK(!d && tx == 0.5 && assperf_escala() == 0.5, "auto 50, busy: whole frame 50 %");
  assperf_teste_definir(AP_ESCALA, AP_RES_100);
  d = quadro(&tx, &bl);
  OK(!d && tx == 1.0 && !assperf_reduzido(), "fixed 100 %");
  assperf_teste_definir(AP_ESCALA, AP_RES_75);
  d = quadro(&tx, &bl);
  OK(!d && tx == 0.75 && !assperf_reduzido(), "fixed 75 %: not an Auto reduction");
  /* Fixed modes leave the busy state; Auto comes back after its 2 s at 100 %. */
  printf(falhas ? "assperf_resolucao: %d FAIL\n" : "assperf_resolucao: ok\n", falhas);
  return falhas != 0;
}
