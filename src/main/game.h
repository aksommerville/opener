#ifndef GAME_H
#define GAME_H

#include "shovel/shovel.h"
#include "opt/r1b/r1b.h"
#include <stdint.h>
#include "ui.h"

#define FBW 64
#define FBH 36
#define TILESIZE 4
#define COLC 16
#define ROWC 9

#define MODE_HELLO 1
#define MODE_PLAY 2

extern const unsigned int palette[16]; // 8 (bg,fg) pairs.
extern const int mapw,maph;
extern const unsigned char map_data[];

extern struct g {

  int pvinput;
  int music_enable;
  int sound_enable;
  int mode;
  
  uint32_t fb[FBW*FBH];
  struct r1b_img32 fbimg;
  
  struct r1b_img1 img_title;
  struct r1b_img1 img_graphics;
  
  // MODE_HELLO
  struct ui_menu menu;
  struct ui_newsfeed newsfeed;
  
  // MODE_PLAY
  int camerax,cameray; // World pixels.
} g;

//TODO audio
#define SFX(tag) ;
#define SONG(tag) sh_ms("\x02\x01",2);

/* Draw text into a 32-bit framebuffer.
 * (dstx,dsty) is the top-left corner of the first glyph.
 * "\n" starts a new line. We don't wrap automatically.
 * Change foreground color temporarily with "\x0c(RGB)", where RGB is three hex digits. "\x0c()" to reset.
 * Text is strictly 8-bit. If you give us UTF-8 or something, we'll still print it bytewise.
 * Bytes outside 0x20..0x7f produce a space.
 * Text is not strictly monospaced; glyphs can have a width of 1, 2, or 3 pixels.
 * Returns horizontal advancement.
 */
int text_render(struct r1b_img32 *dst,int dstx,int dsty,const char *src,int srcc,uint32_t xbgr);

// Width, not including the final gap.
int text_measure(const char *src,int srcc);

int game_reset();
void game_update(double elapsed,int input);
void game_render();

#endif
