// SIGNS & SONGS (owner 19/11/2026, .tpk libass engine).
//
// The dialogue settings of an ASS track (Subtitles > Style: size, opacity,
// and "Dialogue by libass" Off, where the app draws the dialogue as its own
// text) must never touch the signs and songs. libass's own rule (\pos,
// \move, \clip, \iclip, \org, \pbo, a drawing, a Banner/Scroll Effect) misses
// the songs: OP/ED karaoke lines usually have none of those tags.
//
//   1. A track whose NAME says Signs/Songs/S&S/Typesetting/Forced (and not
//      Full/Dialogue) is all signs: no line of it is dialogue.
//   2. A Full/Dialogue track with such a partner in the same file: every
//      line that is also in the partner (same start within 60 ms, same text
//      without tags) is a sign. The partner's lines are read for free from
//      the local relay (mkvfix), the bytes the TV already downloads.
//   3. Everything else: libass's rule, as before.
//
// mkvass sets the track info at the first delivery of a track's lines; the
// relay tap (mkvfix) hands the partner's blocks here. Thread-safe.
#ifndef NV_ASSSINAIS_H
#define NV_ASSSINAIS_H

// mkvass, first lines of the track delivered under subtitle generation `g`:
// the track itself is signs & songs (1/0), the partner track number to read
// from the relay (0 = none), the file's TimestampScale (ns). Forgets the old.
void asssinais_definir(unsigned g, int trilhaSinais, int faixaSinais, unsigned long escalaNs);
// The same track's lines moved to generation `nova` (mkvass reloaded it after
// late fonts): the track info and the partner lines read stay. Only if the
// current generation is `velha`.
void asssinais_regerar(unsigned velha, unsigned nova);
// 1 = generation g's track is a Signs & Songs track (rule 1).
int  asssinais_trilha_sinais(unsigned g);
// 1 = rule 2 has data for generation g (a partner track and some lines).
int  asssinais_ativo(unsigned g);

// mkvfix relay tap: the partner track number now (0 = none).
int  asssinais_relay_faixa(void);
// mkvfix relay tap: one whole BlockGroup / SimpleBlock element of the partner
// track, in a Cluster with timestamp clTs.
void asssinais_bloco(unsigned long clTs, const unsigned char *b, long n);

// Hash of an ASS Text field without override tags ({...}), \N \n \h and runs
// of spaces folded, ASCII lowercased. 0 = no visible text (never matches).
unsigned asssinais_hash(const char *assText);
// Rule 2: the line (start in ms, asssinais_hash of its Text) is in the partner.
int  asssinais_casa(unsigned g, long long inicioMs, unsigned hash);
// Grows with every partner line added (re-check a line marked dialogue).
unsigned asssinais_versao(void);
// libass's own sign rule on a Text field (\pos \move \clip \iclip \org \p..).
int  asssinais_tags(const char *assText);
// The track name rule (lowercase or not): Signs/Songs/S&S/Typesetting/Forced
// and not Full/Dialogue.
int  asssinais_nome(const char *nome);
// Typesetting the split must leave to libass even without \pos: rotation
// (\frx \fry \frz \fr), shear (\fax \fay) or a font change (\fn) in the Text,
// a style named like a sign (asssinais_nome) or a rotated style (Angle).
// Both sides use it: the app drops the line from its dialogue copy
// (legenda.c) and marks the libass event nv_sign (assrender.c), so the line
// is drawn once, by libass, with its font, angle and place.
int  asssinais_tipografia(const char *assText, const char *estilo, double anguloEstilo);

// Log: lines read from the partner, lookups, matches (since the last call).
void asssinais_estatisticas(int *linhas, long *consultas, long *casadas);

#endif
