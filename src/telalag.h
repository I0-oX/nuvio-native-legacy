// GPU QUEUE LIMIT (.tpk, owner 25/10/2026; webOS 14/11/2026).
//
// A GL fence after each frame's last draw, polled (never waited on) at the
// next frames. The limit waits at the start of a frame until the GPU is at
// most one frame behind, but only where it helped on the S95C: Render
// resolution Auto at 75 % on heavy signs (telalag.c, limite).
//
// The .tpk needs an OpenGL ES 3 context (fences are core there), through the
// Tizen EGL loader. webOS asks for an ES 2 context, so there the fences come
// from EGL_KHR_fence_sync (it needs GL_OES_EGL_sync) unless the ES 3 core
// functions are present; both are detected at runtime. Elsewhere every
// function is a no-op.
#ifndef NV_TELALAG_H
#define NV_TELALAG_H

void telalag_quadro_inicio(void);   // GL thread, before the frame's first draw (waits if limited)
void telalag_quadro_fim(void);      // GL thread, right before the swap

#endif
