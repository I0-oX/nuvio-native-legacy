// Signs & Songs matching (src/asssinais.c, owner 19/11/2026). See tests/asssinais.sh.
#include "asssinais.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
#define OK(c, m) do { if (c) printf("ok   %s\n", m); else { printf("FAIL %s\n", m); falhas++; } } while (0)

/* One BlockGroup { Block(track, rel, flags, payload), BlockDuration } */
static long bloco(unsigned char *b, int trk, int rel, const char *payload) {
  unsigned char blk[1200]; long bn = 0, pn = (long)strlen(payload), gn, o = 0;
  blk[bn++] = 0xA1;                         // Block
  blk[bn++] = (unsigned char)(0x40 | ((4 + pn) >> 8)); blk[bn++] = (unsigned char)((4 + pn) & 0xFF);   // 2-byte size
  blk[bn++] = (unsigned char)(0x80 | trk);  // track vint
  blk[bn++] = (unsigned char)((rel >> 8) & 0xFF); blk[bn++] = (unsigned char)(rel & 0xFF);
  blk[bn++] = 0x00;                         // flags
  memcpy(blk + bn, payload, (size_t)pn); bn += pn;
  blk[bn++] = 0x9B; blk[bn++] = 0x81; blk[bn++] = 0x10;   // BlockDuration 16
  gn = bn;
  b[o++] = 0xA0; b[o++] = (unsigned char)(0x40 | (gn >> 8)); b[o++] = (unsigned char)(gn & 0xFF);
  memcpy(b + o, blk, (size_t)gn);
  return o + gn;
}

int main(void) {
  unsigned char b[1400]; long n;
  unsigned h1 = asssinais_hash("{\\fad(200,200)\\k20}Kimi {\\k30}no\\Nnamae"),
           h2 = asssinais_hash("Kimi no namae"), h3 = asssinais_hash("  KIMI  {\\b1}no\\h namae "),
           h4 = asssinais_hash("Kimi no nawa");
  OK(h1 && h1 == h2 && h2 == h3, "hash: tags, \\N, \\h, spaces and case fold");
  OK(h4 != h2, "hash: other text differs");
  OK(asssinais_hash("{\\pos(10,10)}") == 0 && asssinais_hash("") == 0 && asssinais_hash(NULL) == 0, "hash: no visible text = 0");

  OK(asssinais_tags("{\\pos(1,2)}x") && asssinais_tags("{\\an8\\move(1,2,3,4)}x") && asssinais_tags("{\\p1}m 0 0 l 1 1") &&
     asssinais_tags("{\\iclip(1,2,3,4)}x") && asssinais_tags("{\\org(1,1)\\frz5}x"), "tags: libass's sign rule");
  OK(!asssinais_tags("{\\an8\\fad(1,1)}x") && !asssinais_tags("plain \\pos(1,2) outside braces"), "tags: dialogue stays dialogue");

  OK(asssinais_nome("Signs & Songs") && asssinais_nome("English [Signs/Songs]") && asssinais_nome("S&S") &&
     asssinais_nome("Typesetting") && asssinais_nome("Forced"), "name: signs & songs tracks");
  OK(!asssinais_nome("Full") && !asssinais_nome("Dialogue") && !asssinais_nome("Full (incl. Signs & Songs)") &&
     !asssinais_nome("English") && !asssinais_nome(NULL), "name: full / dialogue / plain");
  OK(asssinais_tipografia("{\\fnKomika Hand\\frz8\\an7}Fuku's note", "Default", 0) &&
     asssinais_tipografia("{\\frz-3}x", NULL, 0) && asssinais_tipografia("{\\fr12}x", NULL, 0) &&
     asssinais_tipografia("{\\fry20}x", NULL, 0) && asssinais_tipografia("{\\fax0.1}x", NULL, 0) &&
     asssinais_tipografia("{\\fnArial}x", NULL, 0), "typesetting: rotation, shear, font change");
  OK(asssinais_tipografia("plain", "Sign Note", 0) && asssinais_tipografia("plain", "Default", 7.5),
     "typesetting: sign style name, rotated style");
  OK(!asssinais_tipografia("{\\i1}Hello{\\i0} there", "Default", 0) &&
     !asssinais_tipografia("{\\fad(200,200)\\fs40}x", "Main", 0) && !asssinais_tipografia("a \\frz5 outside", NULL, 0) &&
     !asssinais_tipografia(NULL, NULL, 0), "typesetting: dialogue (italics, fade, size) stays dialogue");

  /* No partner: nothing matches, never active. */
  asssinais_definir(7, 0, 0, 1000000UL);
  OK(asssinais_relay_faixa() == 0 && !asssinais_ativo(7), "no partner: not active");
  n = bloco(b, 3, 100, "1,0,OP,,0,0,0,,{\\k20}Kimi no namae");
  asssinais_bloco(65000, b, n);
  OK(!asssinais_casa(7, 65100, h2), "no partner: a block is ignored");

  /* Partner track 3, TimestampScale 1 ms. */
  asssinais_definir(8, 0, 3, 1000000UL);
  OK(asssinais_relay_faixa() == 3 && !asssinais_trilha_sinais(8), "partner set");
  asssinais_bloco(65000, b, n);              // Cluster 65000 + 100 = 65100 ms
  OK(asssinais_ativo(8), "active after one partner line");
  OK(asssinais_casa(8, 65100, h1), "match: same start, the Full line has other tags");
  OK(asssinais_casa(8, 65150, h2) && asssinais_casa(8, 65045, h2), "match: within 60 ms (also across a 100 ms bucket)");
  OK(!asssinais_casa(8, 65200, h2), "no match: 100 ms off");
  OK(!asssinais_casa(8, 65100, h4), "no match: other text");
  OK(!asssinais_casa(9, 65100, h2), "no match: other subtitle generation");
  n = bloco(b, 4, 100, "2,0,Default,,0,0,0,,Kimi no nawa");
  asssinais_bloco(65000, b, n);
  OK(!asssinais_casa(8, 65100, h4), "a block of another track is not read");
  n = bloco(b, 3, -50, "3,0,Sign,,0,0,0,,{\\pos(1,1)}Shuchiin");
  asssinais_bloco(70000, b, n);
  OK(asssinais_casa(8, 69950, asssinais_hash("Shuchiin")), "negative relative timestamp");
  /* TimestampScale 100 us: cluster 650000 units = 65000 ms. */
  asssinais_definir(10, 0, 3, 100000UL);
  n = bloco(b, 3, 1000, "1,0,OP,,0,0,0,,Kimi no namae");
  asssinais_bloco(650000, b, n);
  OK(asssinais_casa(10, 65100, h2), "TimestampScale applied");
  /* The same generation and partner again (a resumed harvest) keeps the lines. */
  asssinais_definir(10, 0, 3, 100000UL);
  OK(asssinais_casa(10, 65100, h2), "same track again: lines kept");
  /* A Signs & Songs track itself. */
  asssinais_definir(11, 1, 0, 1000000UL);
  OK(asssinais_trilha_sinais(11) && !asssinais_trilha_sinais(10) && !asssinais_ativo(11), "signs & songs track: all signs, no partner");
  /* Many lines: emptied when full, never stuck. */
  asssinais_definir(12, 0, 3, 1000000UL);
  { int i; char p[64];
    for (i = 0; i < 200000; i++) {
      snprintf(p, sizeof p, "%d,0,S,,0,0,0,,line %d", i, i);
      n = bloco(b, 3, 0, p);
      asssinais_bloco((unsigned long)i * 10u, b, n);
    }
    snprintf(p, sizeof p, "line %d", 199999);
    OK(asssinais_casa(12, 1999990, asssinais_hash(p)), "200 000 lines: the newest still found");
  }
  { int l; long q, c; asssinais_estatisticas(&l, &q, &c); OK(l > 0 && q > 0 && c > 0, "stats counted and reset"); }
  printf(falhas ? "asssinais: %d FAIL\n" : "asssinais: ok\n", falhas);
  return falhas != 0;
}
