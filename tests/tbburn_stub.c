// TorBox burn-in (tbburn.h) for the fake-host tests that link src/video_tpk.c
// without the network: burn-in is never possible, so video_tpk.c behaves as
// before it existed.
#include "tbburn.h"
#include <stdint.h>
void tbburn_fonte(const char *u, const char *h, const char *a, uint64_t b) { (void)u; (void)h; (void)a; (void)b; }
void tbburn_ids(const char *u, int s, int i, int f) { (void)u; (void)s; (void)i; (void)f; }
int  tbburn_pode(void) { return 0; }
void tbburn_arquivo_tem_ass(int s) { (void)s; }
void tbburn_pedir(const char *u, int s, int a) { (void)u; (void)s; (void)a; }
int  tbburn_resultado(char *m, unsigned n, char *mo, unsigned mn) {
  if (m && n) m[0] = 0;
  if (mo && mn) mo[0] = 0;
  return TBB_NADA;
}
void tbburn_cancelar(void) {}
int  rede_torbox_em_voo(int *p) { if (p) *p = 0; return 0; }
int  ajustes_leg_queimar(void) { return 0; }
