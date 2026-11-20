// Ver telalag.h.
#include "telalag.h"

#if defined(NV_TPK) || defined(NV_WEBOS)
#include "assperf.h"
#include "gl_compat.h"
#include <string.h>
#if defined(NV_TPK)
#include "tpk_egl.h"
#else
#include <SDL2/SDL.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdio.h>
#endif

/* OpenGL ES 3.0 sync objects, by hand: the link only declares ES 2. */
#define TL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define TL_SYNC_FLUSH_COMMANDS_BIT    0x00000001
#define TL_ALREADY_SIGNALED           0x911A
#define TL_CONDITION_SATISFIED        0x911C
#define TL_WAIT_FAILED                0x911D
typedef void *TlSync;
typedef TlSync (*FnFence)(unsigned, unsigned);
typedef unsigned (*FnWait)(TlSync, unsigned, unsigned long long);
typedef void (*FnDel)(TlSync);
static FnFence pFence;
static FnWait pWait;
static FnDel pDel;
static int estado;   /* 0 not tried, 1 ready, -1 no fences: do nothing */

#define TL_FILA 24   /* frames in flight; at 60 fps that is 400 ms of GPU backlog */
static TlSync fila[TL_FILA];
static int nFila;

#if defined(NV_TPK)
/* The functions come from the Tizen EGL loader (tpk_egl.h): the link does not
 * declare ES 3. */
static int pronto(void) {
  if (estado) return estado > 0;
  estado = -1;
  {
    const char *v = (const char *)glGetString(GL_VERSION);
    if (!v || !strstr(v, "OpenGL ES ") || v[10] < '3') return 0;
  }
  if (!tpkEgl.GetProcAddress) return 0;
  *(void **)&pFence = tpkEgl.GetProcAddress("glFenceSync");
  *(void **)&pWait = tpkEgl.GetProcAddress("glClientWaitSync");
  *(void **)&pDel = tpkEgl.GetProcAddress("glDeleteSync");
  if (!pFence || !pWait || !pDel) return 0;
  estado = 1;
  return 1;
}

static TlSync tl_criar(void) { return pFence(TL_SYNC_GPU_COMMANDS_COMPLETE, 0); }

static int tl_esperar(TlSync s, unsigned flags, unsigned long long ns) {
  unsigned r = pWait(s, flags, ns);
  return r == TL_ALREADY_SIGNALED || r == TL_CONDITION_SATISFIED || r == TL_WAIT_FAILED;
}

static void tl_largar(TlSync s) { pDel(s); }

#else
/* webOS: the window is an ES 2 context (main.c asks for 2.0), so the ES 3 core
 * fences are normally absent. EGL_KHR_fence_sync is the ES 2 route -- for GLES
 * 2 it needs GL_OES_EGL_sync -- and it is loaded by eglGetProcAddress. Both
 * are detected once, at runtime; without either, one line and no limit. */
#ifndef EGL_SYNC_FENCE_KHR
#define EGL_SYNC_FENCE_KHR 0x30F9
#endif
#ifndef EGL_TIMEOUT_EXPIRED_KHR
#define EGL_TIMEOUT_EXPIRED_KHR 0x30F5
#endif
typedef EGLSyncKHR (*FnCriaKhr)(EGLDisplay, EGLenum, const EGLint *);
typedef EGLint (*FnEsperaKhr)(EGLDisplay, EGLSyncKHR, EGLint, EGLTimeKHR);
typedef EGLBoolean (*FnLargaKhr)(EGLDisplay, EGLSyncKHR);
static FnCriaKhr pCriaKhr;
static FnEsperaKhr pEsperaKhr;
static FnLargaKhr pLargaKhr;
static EGLDisplay dpy;
static int modo;   /* 0 none, 1 ES 3 core, 2 EGL_KHR_fence_sync */

static int pronto(void) {
  const char *v;
  if (estado) return estado > 0;
  estado = -1;
  v = (const char *)glGetString(GL_VERSION);
  if (v && !strncmp(v, "OpenGL ES ", 10) && v[10] >= '3') {
    *(void **)&pFence = (void *)SDL_GL_GetProcAddress("glFenceSync");
    *(void **)&pWait = (void *)SDL_GL_GetProcAddress("glClientWaitSync");
    *(void **)&pDel = (void *)SDL_GL_GetProcAddress("glDeleteSync");
    if (pFence && pWait && pDel) { modo = 1; estado = 1; return 1; }
  }
  if (SDL_GL_ExtensionSupported("GL_OES_EGL_sync")) {
    const char *ext;
    dpy = eglGetCurrentDisplay();
    ext = dpy != EGL_NO_DISPLAY ? eglQueryString(dpy, EGL_EXTENSIONS) : NULL;
    if (ext && strstr(ext, "EGL_KHR_fence_sync")) {
      *(void **)&pCriaKhr = (void *)eglGetProcAddress("eglCreateSyncKHR");
      *(void **)&pEsperaKhr = (void *)eglGetProcAddress("eglClientWaitSyncKHR");
      *(void **)&pLargaKhr = (void *)eglGetProcAddress("eglDestroySyncKHR");
      if (pCriaKhr && pEsperaKhr && pLargaKhr) { modo = 2; estado = 1; return 1; }
    }
  }
  printf("[tela] GPU queue limit off: no fences (ES 3 core, or EGL_KHR_fence_sync + GL_OES_EGL_sync)\n");
  fflush(stdout);
  return 0;
}

static TlSync tl_criar(void) {
  if (modo == 1) return pFence(TL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  return pCriaKhr(dpy, EGL_SYNC_FENCE_KHR, NULL);
}

/* EGL answers EGL_TIMEOUT_EXPIRED while the fence is still running; anything
 * else (satisfied, already signalled, or the error EGL_FALSE) is done. */
static int tl_esperar(TlSync s, unsigned flags, unsigned long long ns) {
  if (modo == 1) {
    unsigned r = pWait(s, flags, ns);
    return r == TL_ALREADY_SIGNALED || r == TL_CONDITION_SATISFIED || r == TL_WAIT_FAILED;
  }
  return pEsperaKhr(dpy, s, (EGLint)flags, (EGLTimeKHR)ns) != EGL_TIMEOUT_EXPIRED_KHR;
}

static void tl_largar(TlSync s) {
  if (modo == 1) pDel(s); else pLargaKhr(dpy, s);
}
#endif

static void colher(void) {
  int i = 0;
  while (i < nFila) {
    if (!tl_esperar(fila[i], 0, 0)) break;
    tl_largar(fila[i]);
    i++;
  }
  if (i) { memmove(fila, fila + i, (size_t)(nFila - i) * sizeof fila[0]); nFila -= i; }
}

/* GPU QUEUE LIMIT, by situation (owner 25/10, logs 5 and 7): 1 frame helped
 * only while Render resolution Auto was at 75 % on heavy signs (lag peaks
 * 47-56 ms against 67-117 without); at a fixed 100 % on heavy signs it made
 * them worse (169-217 ms against ~118): there the GL thread itself is the
 * bottleneck and the wait only adds a stall. Light scenes: no difference. */
static int limite(void) {
  return assperf_reduzido();   /* any Auto mode while it is below 100 % (owner 19/11/2026) */
}

void telalag_quadro_inicio(void) {
  int lim;
  if (!pronto()) return;
  colher();
  lim = limite();
  if (lim > 0 && nFila >= lim) {
    /* wait for the frame that leaves only lim-1 still running (100 ms cap) */
    tl_esperar(fila[nFila - lim], TL_SYNC_FLUSH_COMMANDS_BIT, 100000000ull);
    colher();
  }
}

void telalag_quadro_fim(void) {
  TlSync s;
  if (estado <= 0 || !limite()) return;   /* fences only while the limit is on */
  if (nFila == TL_FILA) { tl_largar(fila[0]); memmove(fila, fila + 1, (TL_FILA - 1) * sizeof fila[0]); nFila--; }
  s = tl_criar();
  if (!s) return;
  fila[nFila++] = s;
}

#else
void telalag_quadro_inicio(void) {}
void telalag_quadro_fim(void) {}
#endif
