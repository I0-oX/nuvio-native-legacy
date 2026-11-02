// CAPACIDADES E PARAMETROS DO MOTOR ASS POR PLATAFORMA. Uma decisao so: o
// resto do codigo testa estas macros e nao `NV_TPK || NV_WEBOS` solto, e le os
// numeros daqui pelo nome em vez de carregar um numero medido numa TV.
//
// 1) CAPACIDADES (o que o motor faz). Codigo compartilhado, uma vez so:
//
//   ASS_NOVO       o motor novo (textura Alpha 8-bit com cache, render adiante,
//                  poda de eventos, opacidade do dialogo, escala so do
//                  dialogo). .tpk e webOS.
//   ASS_LIBASS_NV  o libass com os patches de tools/tpk/libass-nv (NEON ARMv7,
//                  fios de blur, API ass_nv_*). .tpk e webOS.
//   ASS_OPCOES     as opcoes da pessoa (assperf.txt) e as linhas do menu
//                  Legendas > Estilo para pistas ASS. .tpk e webOS.
//   ASS_NOVO_TPK   so o que depende do host Samsung: estilos do autor com as
//                  linhas "Preservado pelo ASS", blocos do relay local e a
//                  sincronia com a imagem (legimagem.c). A grade de quadros e
//                  do motor (ASS_NOVO): so a FONTE do fps muda (legimagem_fps
//                  no .tpk, video_quadro_fps no webOS).
//
// 2) PARAMETROS (quanto). Um bloco por plataforma, mais abaixo: SAMSUNG .tpk,
//    LG webOS e o motor antigo. O codigo compartilhado so le os nomes
//    ASSPERF_*; trocar um numero de uma TV nao mexe na outra. Os do webOS
//    comecam iguais aos da S95C porque a mesma biblioteca roda nas duas, mas
//    NENHUM foi medido numa LG: cada um e tema de medicao la.
//
// .wgt, Android e desktop ficam com o motor antigo (tudo 0), exceto pelos
// testes de computador, que ligam NV_ASS_NOVO e usam os numeros da Samsung.
#ifndef NV_ASSPLAT_H
#define NV_ASSPLAT_H

/* ===== 1) CAPACIDADES ====================================================== */
#if defined(NV_TPK) || defined(NV_WEBOS) || defined(NV_ASS_NOVO)
#define ASS_NOVO 1
#else
#define ASS_NOVO 0
#endif

#if defined(NV_TPK) || defined(NV_WEBOS)
#define ASS_LIBASS_NV 1
#define ASS_OPCOES 1
#else
#define ASS_LIBASS_NV 0
#define ASS_OPCOES 0
#endif

#if defined(NV_TPK) || defined(NV_ASS_NOVO)
#define ASS_NOVO_TPK 1
#else
#define ASS_NOVO_TPK 0
#endif

/* ===== 2) PARAMETROS POR PLATAFORMA ========================================
 * Nomes iguais nos tres blocos; so os numeros (e quem os mede) mudam. */

#if defined(NV_WEBOS)
/* ---- LG webOS ------------------------------------------------------------
 * Ainda os numeros da S95C (mesma biblioteca, mesma regra); nao medidos na LG.
 * Sem relay local: nada fica estacionado a frente (ASSPERF_PARQUE 0). */
/* Tempos do motor */
#define ASSPERF_ADIANTE_MS       100          /* render adiante */
#define ASSPERF_PODA_MS          120000L      /* solta evento que acabou ha tanto */
#define ASSPERF_PARQUE           0L           /* blocos do relay estacionados, bytes */
#define ASSPERF_PICO_CARO_MS     40.0         /* render de pico acima disto: mais um quadro a frente */
#define ASSPERF_QUADROS_CAROS    3            /* quadros a frente com render caro */
/* Fontes anexadas: teto de UM bloco = 1/8 da memoria livre, entre MIN e MAX */
#define ASSPERF_FONTES_DINAMICO  1
#define ASSPERF_FONTES_LIVRE_DIV 8
#define ASSPERF_FONTES_MIN       (16ull << 20)
#define ASSPERF_FONTES_MAX       (64ull << 20)
#define ASSPERF_FONTES_VISTAS_MAX    192                       /* fontes distintas no libass antes de reiniciar */
#define ASSPERF_FONTES_ACUM_BYTES    (96u * 1024u * 1024u)     /* e o acumulado delas */
#define ASSPERF_FONTES_COPIA_MAX     32                        /* fontes da entrega atual retidas para refazer o seletor */
#define ASSPERF_FONTES_COPIA_BYTES   (32u * 1024u * 1024u)
/* Fios do blur do libass: nucleos - RESERVA, entre MIN e MAX */
#define ASSPERF_BLUR_RESERVA     1
#define ASSPERF_BLUR_FIOS_MIN    1
#define ASSPERF_BLUR_FIOS_MAX    4
/* Fila de quadros prontos (CPU) e cache de texturas (GPU) */
#define ASSPERF_FILA_MAX         4
#define ASSPERF_FILA_BYTES       (24u * 1024u * 1024u)
#define ASSPERF_QUADRO_IMAGENS   4096                          /* um quadro acima disto perde o excedente */
#define ASSPERF_QUADRO_BYTES     (16u * 1024u * 1024u)
#define ASSPERF_TC_TETO          768
#define ASSPERF_TC_BYTES         (24u * 1024u * 1024u)
#define ASSPERF_TC_RECICLA       2u                            /* quadros GL ate a GPU soltar a textura */
#define ASSPERF_POD_MAX          4000                          /* falas podadas guardadas para o seek para tras */
/* Cena pesada e resolucao Auto (assperf.c) */
#define ASSPERF_CARGA_BALDES     20           /* baldes de 100 ms: janela de 2 s */
#define ASSPERF_CARGA_LIGA       300.0        /* ms de trabalho por segundo */
#define ASSPERF_CARGA_DESLIGA    150.0
#define ASSPERF_CARGA_LEVE_S     3.0
#define ASSPERF_CARGA_OCIOSO_S   8.0
#define ASSPERF_PREV_INICIOS     300
#define ASSPERF_PREV_CHARS       400000L
#define ASSPERF_TELA_SIMPLES     20
#define ASSPERF_AUTO_MIN75_S     5.0
#define ASSPERF_AUTO_MIN100_S    2.0

#elif defined(NV_TPK) || defined(NV_ASS_NOVO)
/* ---- SAMSUNG .tpk (medido na S95C; os testes de computador usam estes) ----- */
/* Tempos do motor */
#define ASSPERF_ADIANTE_MS       100
#define ASSPERF_PODA_MS          120000L
#define ASSPERF_PARQUE           (6L << 20)   /* relay mkvfix: 6 MB estacionados a frente */
#define ASSPERF_PICO_CARO_MS     40.0         /* mascaras do polegar: 50-65 ms no log 15 */
#define ASSPERF_QUADROS_CAROS    3
/* Fontes anexadas: teto de UM bloco = 1/8 da memoria livre, entre MIN e MAX */
#define ASSPERF_FONTES_DINAMICO  1
#define ASSPERF_FONTES_LIVRE_DIV 8
#define ASSPERF_FONTES_MIN       (16ull << 20)
#define ASSPERF_FONTES_MAX       (64ull << 20)
#define ASSPERF_FONTES_VISTAS_MAX    192
#define ASSPERF_FONTES_ACUM_BYTES    (96u * 1024u * 1024u)
#define ASSPERF_FONTES_COPIA_MAX     32
#define ASSPERF_FONTES_COPIA_BYTES   (32u * 1024u * 1024u)
/* Fios do blur do libass: nucleos - RESERVA (um fica para a UI e o video) */
#define ASSPERF_BLUR_RESERVA     1
#define ASSPERF_BLUR_FIOS_MIN    1
#define ASSPERF_BLUR_FIOS_MAX    4
/* Fila de quadros prontos (CPU) e cache de texturas (GPU) */
#define ASSPERF_FILA_MAX         4
#define ASSPERF_FILA_BYTES       (24u * 1024u * 1024u)
#define ASSPERF_QUADRO_IMAGENS   4096
#define ASSPERF_QUADRO_BYTES     (16u * 1024u * 1024u)
#define ASSPERF_TC_TETO          768
#define ASSPERF_TC_BYTES         (24u * 1024u * 1024u)
#define ASSPERF_TC_RECICLA       2u
#define ASSPERF_POD_MAX          4000
/* Cena pesada e resolucao Auto (assperf.c): logs 9, 12 e 15 do dono, 26/10 */
#define ASSPERF_CARGA_BALDES     20
#define ASSPERF_CARGA_LIGA       300.0
#define ASSPERF_CARGA_DESLIGA    150.0
#define ASSPERF_CARGA_LEVE_S     3.0
#define ASSPERF_CARGA_OCIOSO_S   8.0
#define ASSPERF_PREV_INICIOS     300
#define ASSPERF_PREV_CHARS       400000L
#define ASSPERF_TELA_SIMPLES     20
#define ASSPERF_AUTO_MIN75_S     5.0
#define ASSPERF_AUTO_MIN100_S    2.0

#else
/* ---- .wgt, Android, desktop: o motor antigo (ASS_NOVO 0) ------------------
 * Nada de render adiante, poda ou relay; teto de fontes fixo em 16 MB. A fila
 * e os tetos de quadro sao os de sempre; as contas de cena pesada nao rodam
 * (assperf_get devolve 0) mas o assperf.c as compila. */
#define ASSPERF_ADIANTE_MS       0
#define ASSPERF_PODA_MS          0L
#define ASSPERF_PARQUE           0L
#define ASSPERF_PICO_CARO_MS     40.0
#define ASSPERF_QUADROS_CAROS    3
#define ASSPERF_FONTES_DINAMICO  0
#define ASSPERF_FONTES_LIVRE_DIV 8
#define ASSPERF_FONTES_MIN       (16ull << 20)   /* o teto fixo */
#define ASSPERF_FONTES_MAX       (16ull << 20)
#define ASSPERF_FONTES_VISTAS_MAX    192
#define ASSPERF_FONTES_ACUM_BYTES    (96u * 1024u * 1024u)
#define ASSPERF_FONTES_COPIA_MAX     32
#define ASSPERF_FONTES_COPIA_BYTES   (32u * 1024u * 1024u)
#define ASSPERF_FILA_MAX         4
#define ASSPERF_FILA_BYTES       (24u * 1024u * 1024u)
#define ASSPERF_QUADRO_IMAGENS   4096
#define ASSPERF_QUADRO_BYTES     (16u * 1024u * 1024u)
#define ASSPERF_POD_MAX          4000
#define ASSPERF_CARGA_BALDES     20
#define ASSPERF_CARGA_LIGA       300.0
#define ASSPERF_CARGA_DESLIGA    150.0
#define ASSPERF_CARGA_LEVE_S     3.0
#define ASSPERF_CARGA_OCIOSO_S   8.0
#define ASSPERF_PREV_INICIOS     300
#define ASSPERF_PREV_CHARS       400000L
#define ASSPERF_TELA_SIMPLES     20
#define ASSPERF_AUTO_MIN75_S     5.0
#define ASSPERF_AUTO_MIN100_S    2.0
#endif

#endif
