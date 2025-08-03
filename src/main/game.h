#ifndef GAME_H
#define GAME_H

#include "shovel/shovel.h"
#include "opt/r1b/r1b.h"
#include <stdint.h>
#include "ui.h"
#include "sprite/sprite.h"

#define FBW 64
#define FBH 36
#define TILESIZE 4
#define SPRITESIZE 8
#define COLC 16 /* Framebuffer size, phrased in tiles. */
#define ROWC 9

#define MODE_HELLO 1
#define MODE_PLAY 2

#define SPRITE_LIMIT 64 /* in the whole world */
#define HEROPATH_LIMIT 64 /* Takes roughly 10 frames to cross one meter, so the 4-meter parade should fit easily in 64. */

extern const unsigned int palette[16]; // 8 (bg,fg) pairs.
extern const int mapw,maph;
extern const unsigned char map_data[];

extern struct g {

  int input,pvinput;
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
  struct sprite spritev[SPRITE_LIMIT];
  int spritec;
  struct pathpos { int16_t x,y; } heropath[HEROPATH_LIMIT]; // Circular. Coordinates in world pixels.
  int heropathp; // Position of the oldest entry. She's most recently at p-1. Advances only when the hero moves.
  
  // Key sprites idenitifed at the start of each game_update. All are WEAK and OPTIONAL.
  struct sprite *hero;
} g;

#define SFX(tag) sh_ms(SFX_##tag,sizeof(SFX_##tag)-1);
#define SONG(tag) sh_ms("\x02" SONG_##tag,2);
// Songs are listed in src/audio/main.c. Tags are the stem of the original file name.
#define SONG_none "\x00"
#define SONG_across_the_scrubby_moors "\x01"
// Sounds, you provide the entire message: 0x01,noteida 0..63,noteidz 0..63,level 0..31,duration 16ms
#define SFX_uimotion "\x01\x20\x30\x08\x05"

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
void game_update(double elapsed);
void game_render();

struct sprite *sprite_spawn(const struct sprite_type *type,int x,int y,uint32_t arg);

/* stdlib functions that we either get from real libc, or main.c implements them for web.
 */
#if USE_web
  void *memset(void *s, int n, long c);
  void *memcpy(void *dst,const void *src,unsigned long c);
  void *memmove(void *dst,const void *src,int c);
  int memcmp(const void *a,const void *b,int c);
  static inline void fprintf(void *f,const char *fmt,...) {}
  #define stderr 0
#else
  #include <string.h>
  #include <stdio.h>
#endif

#endif
