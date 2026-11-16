// gfx_marca_lote (16/11/2026) num GL de verdade (GLES2 do Mesa, janela
// escondida): 300 marcas de um atlas com moldura repetida, num lote so, contra
// a conta feita na CPU (amostra bilinear com CLAMP_TO_EDGE da textura de cada
// uma, mistura alfa na ordem), em 1:1, ampliado 2x e com a escala da
// interface 1,5 + opacidade do grupo 0,6. O caminho de um gfx_rect por marca
// e medido contra a mesma conta (so informa: o mediump do programa dele erra a
// coordenada de textura no llvmpipe). Depois do lote, gfx_rect segue
// desenhando igual (o buffer do quad e o atributo 0 voltaram).
//
//   bash tests/gfx_lote_shot.sh
#include "gfx.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 1920
#define H 1080
#define N 300
static GLuint fbo, fboTex;
static int falhas;
static void ok(int c, const char *o) { printf("%s %s\n", c ? "ok   " : "FALHA", o); if (!c) falhas++; }

typedef struct { int w, h; unsigned char *px; GLuint tex; int ax, ay; float x, y, esc, r, g, b, a; } Marca;
static Marca m[N];

static void ler(unsigned char *dst) {
  glFinish(); glBindFramebuffer(GL_FRAMEBUFFER, fbo); glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, dst);
}
static void limpar(void) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo); glViewport(0, 0, W, H);
  glClearColor(0.1f, 0.12f, 0.16f, 1); glClear(GL_COLOR_BUFFER_BIT);
}
/* A conta da CPU: o que o GL deveria mostrar. */
static float bil(const Marca *q, float u, float v) {   // u, v em texels
  float x = u - 0.5f, y = v - 0.5f; int x0 = (int)floorf(x), y0 = (int)floorf(y), x1 = x0 + 1, y1 = y0 + 1;
  float fx = x - x0, fy = y - y0;
#define CL(a, n) ((a) < 0 ? 0 : (a) >= (n) ? (n) - 1 : (a))
#define T(xx, yy) (q->px[CL(yy, q->h) * q->w + CL(xx, q->w)] / 255.0f)
  return (T(x0, y0) * (1 - fx) + T(x1, y0) * fx) * (1 - fy) + (T(x0, y1) * (1 - fx) + T(x1, y1) * fx) * fy;
#undef T
#undef CL
}
static void referencia(unsigned char *dst, const GfxMarcaItem *it, float escUi, float grupo) {
  static float f[H][W][3]; int i, x, y;
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) { f[y][x][0] = 0.1f; f[y][x][1] = 0.12f; f[y][x][2] = 0.16f; }
  for (i = 0; i < N; i++) {
    float X0 = it[i].x * escUi, Y0 = it[i].y * escUi, X1 = X0 + it[i].w * escUi, Y1 = Y0 + it[i].h * escUi;
    for (y = (int)floorf(Y0); y <= (int)Y1 && y < H; y++) for (x = (int)floorf(X0); x <= (int)X1 && x < W; x++) {
      float cx = x + 0.5f, cy = y + 0.5f, a; int c;
      /* Regra de borda do GL com origem embaixo: centro na borda esquerda ou
       * na de BAIXO da tela entra; na direita ou na de cima, nao. */
      if (cx < X0 || cx >= X1 || cy <= Y0 || cy > Y1) continue;
      a = it[i].a * grupo * bil(&m[i], (cx - X0) / (X1 - X0) * m[i].w, (cy - Y0) / (Y1 - Y0) * m[i].h);
      for (c = 0; c < 3; c++) f[y][x][c] = f[y][x][c] * (1 - a) + (&it[i].r)[c] * a;
    }
  }
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) { int c; unsigned char *p = dst + ((long)(H - 1 - y) * W + x) * 4;
    for (c = 0; c < 3; c++) p[c] = (unsigned char)(f[y][x][c] * 255.0f + 0.5f); p[3] = 255; }
}
static int dif_rgb(const unsigned char *a, const unsigned char *b, int tol, long *n) {
  long i; int mx = 0; *n = 0;
  for (i = 0; i < (long)W * H * 4; i++) { int d; if (i % 4 == 3) continue; d = abs((int)a[i] - (int)b[i]); if (d > mx) mx = d; if (d > tol) (*n)++; }
  return mx;
}

static unsigned rnd(void) { static unsigned s = 160611; s = s * 1103515245u + 12345u; return s >> 8; }

int main(void) {
  GLuint atlas; int lado = 2048, px = 0, py = 0, ph = 0, i, j, k, rodada;
  unsigned char *pad = malloc(200 * 200), *A = malloc((size_t)W * H * 4), *B = malloc((size_t)W * H * 4), *R = malloc((size_t)W * H * 4);
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_Window *win = SDL_CreateWindow("lote", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN); assert(win);
  SDL_GLContext gl = SDL_GL_CreateContext(win); assert(gl);
  printf("GL: %s | %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
  glGenTextures(1, &fboTex); glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, W, H); gfx_tamanho_alvo(W, H); assert(gfx_iniciar());

  /* Marcas como as do libass: tamanhos variados, alfa suave, algumas com borda
   * cheia (caixa opaca do BorderStyle 3), cores em degraus de 1/255. */
  glGenTextures(1, &atlas); glBindTexture(GL_TEXTURE_2D, atlas);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, lado, lado, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (i = 0; i < N; i++) {
    Marca *q = &m[i]; int caixa = i % 7 == 0, pw, phh;
    q->w = 4 + (int)(rnd() % 100); q->h = 4 + (int)(rnd() % 60);
    q->px = malloc((size_t)q->w * q->h);
    for (j = 0; j < q->h; j++) for (k = 0; k < q->w; k++) {
      float dx = (k + 0.5f) / q->w - 0.5f, dy = (j + 0.5f) / q->h - 0.5f;
      float v = caixa ? 1.0f : 1.0f - 2.2f * sqrtf(dx * dx + dy * dy) + 0.25f * sinf(k * 0.7f + j * 0.3f);
      q->px[j * q->w + k] = (unsigned char)(v <= 0 ? 0 : v >= 1 ? 255 : (int)(v * 255));
    }
    q->x = (float)(rnd() % 566 * 3); q->y = (float)(rnd() % 270 * 3);   // multiplos de 3: /1,5 x 1,5 exato
    q->r = (float)(rnd() % 256) / 255.0f; q->g = (float)(rnd() % 256) / 255.0f; q->b = (float)(rnd() % 256) / 255.0f;
    q->a = (float)(64 + rnd() % 192) / 255.0f;
    glGenTextures(1, &q->tex); glBindTexture(GL_TEXTURE_2D, q->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, q->w, q->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, q->px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* No atlas: prateleira e moldura repetida, como o assrender.c. */
    pw = q->w + 2; phh = q->h + 2;
    if (px + pw > lado) { py += ph; px = 0; ph = 0; }
    assert(py + phh <= lado);
    for (j = 0; j < q->h; j++) { unsigned char *d = pad + (j + 1) * pw; memcpy(d + 1, q->px + j * q->w, q->w); d[0] = d[1]; d[pw - 1] = d[pw - 2]; }
    memcpy(pad, pad + pw, pw); memcpy(pad + (phh - 1) * pw, pad + (phh - 2) * pw, pw);
    glBindTexture(GL_TEXTURE_2D, atlas);
    glTexSubImage2D(GL_TEXTURE_2D, 0, px, py, pw, phh, GL_ALPHA, GL_UNSIGNED_BYTE, pad);
    q->ax = px + 1; q->ay = py + 1; px += pw; if (phh > ph) ph = phh;
  }
  gfx_tex_esquecer(0);

  for (rodada = 0; rodada < 3; rodada++) {
    /* 0: 1:1; 1: ampliado 2x (Render resolution menor); 2: escala da UI 1,5 e
     * opacidade do grupo 0,6. Bordas em posicoes que o mediump (fp16) do
     * caminho um a um representa exatas: a comparacao e do lote, nao do fp16. */
    float esc = rodada == 1 ? 0.5f : 1.0f, escUi = rodada == 2 ? 1.5f : 1.0f, grupo = rodada == 2 ? 0.6f : 1.0f;
    static GfxMarcaItem it[N];
    long nDif; int mx;
    float escAnt;
    const char *nome[3] = { "1:1", "ampliado 2x", "escala da UI 1,5 + opacidade do grupo 0,6" };
    for (i = 0; i < N; i++) {
      Marca *q = &m[i]; GfxMarcaItem *t = &it[i];
      t->x = q->x / escUi; t->y = q->y / escUi;
      t->w = q->w / esc; t->h = q->h / esc;
      t->u0 = (float)q->ax / lado; t->v0 = (float)q->ay / lado;
      t->u1 = (float)(q->ax + q->w) / lado; t->v1 = (float)(q->ay + q->h) / lado;
      t->r = q->r; t->g = q->g; t->b = q->b; t->a = q->a;
    }
    escAnt = gfx_escala_entrar(); gfx_escala_sair(escUi);
    gfx_opacidade_grupo = grupo;
    limpar();
    for (i = 0; i < N; i++)
      gfx_rect((GfxRect){ it[i].x, it[i].y, it[i].w, it[i].h }, m[i].tex, GFX_MARCA, 0, 0, 0, 0, it[i].r, it[i].g, it[i].b, it[i].a);
    ler(A);
    limpar();
    ok(gfx_marca_lote(atlas, it, N) == 1, "gfx_marca_lote aceitou o lote");
    ler(B);
    gfx_opacidade_grupo = 1.0f; gfx_escala_sair(escAnt);
    referencia(R, it, escUi, grupo);
    { long nA; int mxA = dif_rgb(A, R, 3, &nA);
      mx = dif_rgb(B, R, 4, &nDif);
      printf("    %s: lote x CPU %d/255 (%ld canais acima de 4); um a um x CPU %d/255 (%ld acima de 3)\n", nome[rodada], mx, nDif, mxA, nA);
      ok(nDif == 0, "lote == a conta da CPU (ate 4/255: o filtro do GL em 8 bits)"); }
  }

  /* Depois do lote o quad volta: retangulo e textura pelo gfx_rect. */
  limpar();
  gfx_cor((GfxRect){ 100, 100, 300, 200 }, 0, 0.8f, 0.2f, 0.4f, 1.0f);
  gfx_rect((GfxRect){ 500, 100, 300, 200 }, m[0].tex, GFX_MARCA, 0, 0, 0, 0, 1, 1, 1, 1);
  ler(A);
  limpar();
  { GfxMarcaItem um = { 900, 900, 50, 50, 0, 0, 0.01f, 0.01f, 1, 0, 0, 1 }; gfx_marca_lote(atlas, &um, 1); }
  limpar();
  gfx_cor((GfxRect){ 100, 100, 300, 200 }, 0, 0.8f, 0.2f, 0.4f, 1.0f);
  gfx_rect((GfxRect){ 500, 100, 300, 200 }, m[0].tex, GFX_MARCA, 0, 0, 0, 0, 1, 1, 1, 1);
  ler(B);
  { long nDif; int mx = dif_rgb(A, B, 0, &nDif); ok(mx == 0, "gfx_rect depois do lote desenha igual"); }
  ok(glGetError() == GL_NO_ERROR, "nenhum erro GL");

  gfx_encerrar();
  printf("\n%s (%d falha%s)\n", falhas ? "FALHOU" : "gfx_lote_shot: ok", falhas, falhas == 1 ? "" : "s");
  return falhas ? 1 : 0;
}
