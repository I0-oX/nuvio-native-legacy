#ifndef NUVIO_TBBURN_H
#define NUVIO_TBBURN_H
// TORBOX BURN-IN (owner, 12/11/2026). An embedded ASS/SSA track on a TorBox
// source plays as TorBox's transcoded HLS stream with the track burned into
// the picture: frame-exact, and the TV's picture processing applies to it.
// TorBox transcodes for Pro accounts only, so it needs Settings > Debrid >
// "TorBox plan" = Pro, "Burn in ASS with TorBox" On (default) and a TorBox key.
//
// The playing URL stays the ORIGINAL file for everything else (sync, resume,
// chapters, thumbnails: video_url_atual() in video_tpk.c); only the URL handed
// to the TV player changes. This module finds the file in the account, asks
// TorBox for the stream and logs everything with the key redacted.
//
// TorBox's web player URL is .../stream/<id>/<resolution>/<subtitle>/<audio>/
// playlist.m3u8 (owner's screenshot, 12/11): each index counts only the tracks
// of its own kind, from 0, in file order.
#include <stdint.h>

enum { TBB_NADA = 0, TBB_BUSCANDO, TBB_PRONTO, TBB_FALHOU };

// The source the player opened (player.c, after video_tocar). `url` is the
// URL the player plays (video_url_atual()); the rest comes from the add-on.
void tbburn_fonte(const char *url, const char *hash, const char *arquivo, uint64_t bytes);

// The exact TorBox file of `url` when it is already known (a file picked in
// "files of the source": video_tpk_cdn_arquivo). Skips the account lookup.
void tbburn_ids(const char *url, int usenet, int item, int arquivo);

// Settings On + plan Pro + a TorBox key, not refused by TorBox this app
// session, and not switched Off for this playback.
int  tbburn_pode(void);
// The in-playback "Burn in" row: -1 = follow Settings, 0 = Off, 1 = On. Lasts
// for the session (Next episodes too); tbburn_sessao_nova clears it.
int  tbburn_escolha(void);
void tbburn_definir_escolha(int v);
void tbburn_sessao_nova(void);
// The next close is the step to the next episode: keep the playback choice.
void tbburn_manter_proximo(void);
// The row is worth showing: plan Pro, a key, and the file is on TorBox (found)
// or not looked up yet. Set by video_tpk.c when the file has an embedded ASS.
int  tbburn_linha_visivel(void);
void tbburn_arquivo_tem_ass(int sim);

// Asks TorBox for the burned stream of `url` with subtitle `sub` and audio
// `audio` (indices among tracks of their kind). Async; one at a time.
void tbburn_pedir(const char *url, int sub, int audio);
// TBB_*; on TBB_PRONTO `m3u8` holds the playlist URL (with TorBox's token: never
// log it raw). `motivo` (may be NULL) says why on TBB_FALHOU.
int  tbburn_resultado(char *m3u8, unsigned n, char *motivo, unsigned mn);
void tbburn_cancelar(void);
// TorBox refused the plan (not Pro): burn-in stays off for this app session.
int  tbburn_recusado(void);

#ifdef NV_TPK
// video_tpk.c. _queimar: the subtitle `i` (menu index, -1 = none/add-on) was
// just chosen; 1 = it is burned in (or being asked for), the caller draws
// nothing. _queimada: the burned menu entry (-1 none). _voltou: 1 once after
// burn-in gave up on this source (the caller re-picks the subtitle).
int  video_tpk_queimar(int i);
int  video_tpk_queimada(void);
int  video_tpk_queimar_voltou(void);
#endif

// ---- pure (tests) ----------------------------------------------------------
// First http(s) URL ending a path in ".m3u8" anywhere in `corpo` (JSON string
// values unescaped: \/ \" \\). 1 found.
int  tbburn_achar_m3u8(const char *corpo, char *out, unsigned n);
// The string value of the first "chave": "..." anywhere in `corpo`. 1 found.
int  tbburn_json_texto(const char *corpo, const char *chave, char *out, unsigned n);
// Whether TorBox's answer says the account plan can't do this.
int  tbburn_eh_recusa_plano(int st, const char *corpo);
// The createstream route for item/file/sub/audio (no key in it).
void tbburn_rota_criar(char *out, unsigned n, int item, int arquivo, int usenet, int sub, int audio);

#endif
