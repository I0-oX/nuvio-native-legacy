// Faixa ASS (tipicamente a FORCED/Signs de um remux) cujos blocos estao com
// ContentEncoding do Matroska: header stripping ou zlib. Antes o mkvass aceitava
// a faixa como ASS e entregava os bytes crus ao overlay: nada na tela e nenhum
// erro. Agora desfaz a codificacao (e devolve a faixa a TV quando nao sabe).
//
// Uso: ./test <zlib|strip|cifra>
#include "mkvass.h"
#include "legenda.h"
#include "dados.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
const char *i18n(const char *s) { return s; }
#define FALA "Sign: forced line, compressed block"
#define BUFMAX (256 * 1024)

typedef struct { unsigned char *b; long n; } Buf;
static unsigned char *arq; static long arqN;
static int pedidos;
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static Buf nova(void) { Buf b; b.b = calloc(1, BUFMAX); b.n = 0; assert(b.b); return b; }
static void bytes(Buf *b, const void *p, long n) {
  assert(b->n + n <= BUFMAX);
  memcpy(b->b + b->n, p, (size_t)n); b->n += n;
}
static void id(Buf *b, unsigned long v) {
  unsigned char t[4]; int n = v > 0xFFFFFF ? 4 : v > 0xFFFF ? 3 : v > 0xFF ? 2 : 1, i;
  for (i = 0; i < n; i++) t[i] = (unsigned char)(v >> (8 * (n - 1 - i)));
  bytes(b, t, n);
}
static void tam8(Buf *b, long long v) {
  unsigned char t[8]; int i;
  t[0] = 0x01;
  for (i = 1; i < 8; i++) t[i] = (unsigned char)(v >> (8 * (7 - i)));
  bytes(b, t, 8);
}
static void uintEl(Buf *b, unsigned long eid, unsigned long long v, int n) {
  unsigned char t[8]; int i;
  id(b, eid); { unsigned char s = (unsigned char)(0x80 | n); bytes(b, &s, 1); }
  for (i = 0; i < n; i++) t[i] = (unsigned char)(v >> (8 * (n - 1 - i)));
  bytes(b, t, n);
}
static void strEl(Buf *b, unsigned long eid, const char *s) {
  unsigned char z = (unsigned char)(0x80 | strlen(s));
  id(b, eid); bytes(b, &z, 1); bytes(b, s, (long)strlen(s));
}
static void mestre(Buf *b, unsigned long eid, const Buf *corpo) {
  id(b, eid); tam8(b, corpo->n); bytes(b, corpo->b, corpo->n);
}
static void binEl(Buf *b, unsigned long eid, const void *p, long n) {
  id(b, eid); tam8(b, n); bytes(b, p, n);
}
static void seekEl(Buf *sh, const unsigned char alvo[4], long long pos) {
  Buf s = nova();
  id(&s, 0x53AB); { unsigned char z = 0x84; bytes(&s, &z, 1); bytes(&s, alvo, 4); }
  uintEl(&s, 0x53AC, (unsigned long long)pos, 8);
  mestre(sh, 0x4DBB, &s); free(s.b);
}

static long idLen(unsigned long v) { return v > 0xFFFFFF ? 4 : v > 0xFFFF ? 3 : v > 0xFF ? 2 : 1; }

static void montar(const char *modo) {
  Buf f = nova(), ebml = nova(), info = nova(), seekHead = nova(), tracks = nova(), v = nova(), s = nova();
  Buf corpo = nova(), block = nova(), grupo = nova(), pos = nova(), ponto = nova(), lista = nova(), cues = nova(), clu = nova();
  Buf codif = nova(), enc = nova(), comp = nova();
  static const unsigned char TR[4] = { 0x16, 0x54, 0xAE, 0x6B }, CU[4] = { 0x1C, 0x53, 0xBB, 0x6B };
  long segIniL, lenPos, trAbs, cuAbs, cluAbs, relBloco;
  const char *priv = "[Script Info]\r\nScriptType: v4.00+\r\nPlayResX: 1920\r\nPlayResY: 1080\r\n\r\n"
    "[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n"
    "Style: Signs,Arial,48,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,1\r\n"
    "\r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
  const char *evento = "0,0,Signs,,0,0,0,," FALA;
  unsigned char h[4] = { 0x82, 0, 0, 0 };

  uintEl(&ebml, 0x4286, 1, 1); strEl(&ebml, 0x4282, "matroska"); mestre(&f, 0x1A45DFA3UL, &ebml);
  id(&f, 0x18538067UL); lenPos = f.n; tam8(&f, 0); segIniL = f.n;
  uintEl(&info, 0x2AD7B1, 1000000, 4);
  seekEl(&seekHead, TR, 0); seekEl(&seekHead, CU, 0);
  trAbs = f.n + idLen(0x114D9B74UL) + 8 + seekHead.n + idLen(0x1549A966UL) + 8 + info.n;

  uintEl(&v, 0xD7, 1, 1); uintEl(&v, 0x83, 1, 1); strEl(&v, 0x86, "V_MPEG4/ISO/AVC");
  uintEl(&s, 0xD7, 2, 1); uintEl(&s, 0x83, 17, 1); strEl(&s, 0x86, "S_TEXT/ASS");
  strEl(&s, 0x22B59C, "eng"); strEl(&s, 0x536E, "Signs");
  binEl(&s, 0x63A2, priv, (long)strlen(priv));
  // ContentEncodings > ContentEncoding > (Scope=1 Block, Type=0 compressao | cifra)
  uintEl(&enc, 0x5031, 0, 1); uintEl(&enc, 0x5032, 1, 1);
  if (!strcmp(modo, "cifra")) {
    uintEl(&enc, 0x5033, 1, 1);
    { Buf e = nova(); uintEl(&e, 0x47E1, 1, 1); mestre(&enc, 0x5035, &e); free(e.b); }
  } else {
    uintEl(&enc, 0x5033, 0, 1);
    if (!strcmp(modo, "strip")) {
      // Header stripping: o prefixo comum (aqui o ReadOrder "0,") some do bloco.
      uintEl(&comp, 0x4254, 3, 1); binEl(&comp, 0x4255, "0,", 2);
    } else uintEl(&comp, 0x4254, 0, 1);
    mestre(&enc, 0x5034, &comp);
  }
  mestre(&codif, 0x6240, &enc); mestre(&s, 0x6D80, &codif);
  mestre(&tracks, 0xAE, &v); mestre(&tracks, 0xAE, &s);
  cluAbs = trAbs + idLen(0x1654AE6BUL) + 8 + tracks.n;

  uintEl(&corpo, 0xE7, 60000, 4);
  relBloco = corpo.n;
  bytes(&block, h, 4);
  if (!strcmp(modo, "strip")) bytes(&block, evento + 2, (long)strlen(evento) - 2);
  else if (!strcmp(modo, "zlib")) {
    uLongf n = compressBound((uLong)strlen(evento)); Bytef *z = malloc(n);
    assert(z && compress(z, &n, (const Bytef *)evento, (uLong)strlen(evento)) == Z_OK);
    bytes(&block, z, (long)n); free(z);
  } else bytes(&block, "\x01\x02\x03\x04\x05\x06", 6);
  mestre(&grupo, 0xA1, &block); uintEl(&grupo, 0x9B, 2000, 2);
  mestre(&corpo, 0xA0, &grupo); mestre(&clu, 0x1F43B675UL, &corpo);
  cuAbs = cluAbs + clu.n;

  uintEl(&pos, 0xF7, 2, 1);
  uintEl(&pos, 0xF1, (unsigned long long)(cluAbs - segIniL), 8);
  uintEl(&pos, 0xF0, (unsigned long long)relBloco, 2);
  uintEl(&ponto, 0xB3, 60000, 4);
  mestre(&ponto, 0xB7, &pos); mestre(&lista, 0xBB, &ponto); mestre(&cues, 0x1C53BB6BUL, &lista);

  seekHead.n = 0;
  seekEl(&seekHead, TR, trAbs - segIniL); seekEl(&seekHead, CU, cuAbs - segIniL);
  mestre(&f, 0x114D9B74UL, &seekHead); mestre(&f, 0x1549A966UL, &info);
  assert(f.n == trAbs);
  mestre(&f, 0x1654AE6BUL, &tracks);
  assert(f.n == cluAbs);
  bytes(&f, clu.b, clu.n);
  assert(f.n == cuAbs);
  bytes(&f, cues.b, cues.n);
  { Buf vz = nova(); static unsigned char z[16384]; binEl(&vz, 0xEC, z, sizeof z); bytes(&f, vz.b, vz.n); free(vz.b); }
  { Buf t = nova(); tam8(&t, f.n - segIniL); memcpy(f.b + lenPos, t.b, 8); free(t.b); }
  arq = f.b; arqN = f.n;
}

char *rede_baixar(const char *url, int s) { (void)url; (void)s; return NULL; }
char *rede_baixar_bin(const char *url, int s, long *n) { (void)url; (void)s; (void)n; return NULL; }
const char *rede_url_publica(const char *url, char *dst, unsigned tam) {
  (void)url; snprintf(dst, tam, "local"); return dst;
}
long rede_corte_host(const char *url) { (void)url; return 0; }
int rede_resto_recusado(void) { return 0; }

char *rede_baixar_trecho_st(const char *url, int s, long long ini, long long fim, long *tam,
                            int *status, int *erro, char *final, unsigned tamFinal) {
  long n; char *p;
  (void)s;
  *tam = 0;
  if (status) *status = 0;
  if (erro) *erro = 0;
  if (final && tamFinal) snprintf(final, tamFinal, "%s", url);
  pthread_mutex_lock(&trava);
  pedidos++;
  pthread_mutex_unlock(&trava);
  if (ini < 0 || fim < ini || ini >= arqN) { if (status) *status = 416; return NULL; }
  if (fim >= arqN) fim = arqN - 1;
  n = (long)(fim - ini + 1);
  p = calloc(1, (size_t)n + 1); assert(p);
  memcpy(p, arq + ini, (size_t)n);
  *tam = n;
  if (status) *status = 206;
  return p;
}


static int esperar(void) {
  for (int i = 0; i < 2000; i++) {
    int e = mkvass_estado();
    if (e == MKVASS_COMPLETO || e >= MKVASS_NOGO) return e;
    mkvass_passo(60.0);
    usleep(10000);
  }
  return mkvass_estado();
}

int main(int argc, char **argv) {
  const char *modo = argc > 1 ? argv[1] : "zlib";
  int e; LegendaCue cue[4];
  montar(modo);
  dados_iniciar(".");
  mkvass_iniciar_ordinal("https://example.test/forced.mkv", 0);
  e = esperar();
  printf("%s: estado %d, %d pedidos\n", modo, e, pedidos);
  if (!strcmp(modo, "cifra")) {
    assert(e == MKVASS_NOGO_FAIXA);          // nao sabemos desfazer: a TV desenha
  } else {
    assert(e == MKVASS_COMPLETO);
    assert(legenda_cues(60.5, 0, cue, 4) == 1);
    assert(strstr(cue[0].texto, FALA));      // antes: lixo ou nada, sem erro
  }
  mkvass_parar();
  printf("mkvass_codificada %s: ok\n", modo);
  return 0;
}
