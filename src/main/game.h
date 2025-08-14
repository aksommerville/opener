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
#define PAN_SPEED 4 /* px/frame, must be greater than one. */

#define MODE_HELLO 1
#define MODE_PLAY 2
#define MODE_DIALOGUE 3
#define MODE_GAMEOVER 4

#define SPRITE_LIMIT 64 /* in the whole world */
#define HEROPATH_LIMIT 64 /* Takes roughly 10 frames to cross one meter, so the 4-meter parade should fit easily in 64. */
#define DLOGTEXT_LIMIT 128

extern const unsigned int palette[16]; // 8 (bg,fg) pairs.
extern const int mapw,maph;
extern const unsigned char map_data[];
extern unsigned char map[]; // Parallel to (map_data) but writeable.

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
  int cameradstx,cameradsty;
  struct sprite spritev[SPRITE_LIMIT];
  int spritec;
  struct pathpos { int16_t x,y; } heropath[HEROPATH_LIMIT]; // Circular. Coordinates in world pixels.
  int heropathp; // Position of the oldest entry. She's most recently at p-1. Advances only when the hero moves.
  int animalc; // 0..4
  int key; // 0..1, doesn't matter which
  
  // Key sprites idenitifed at the start of each game_update. All are WEAK and OPTIONAL.
  struct sprite *hero;
  
  // MODE_DIALOGUE
  char dlogtext[DLOGTEXT_LIMIT];
  int dlogtextp; // How many characters rendered.
  int dlogtextc; // How many total.
  double dlogclock; // Counts down to next advancement of dlogtextp.
  int dlogframe;
  double dloganimclock;
  int dlogsrcx,dlogsrcy;
  uint32_t dlogbg,dlogfg;
  
  // MODE_GAMEOVER
  double gameover_clock;
} g;

#define SFX(tag) sh_ms(SFX_##tag,sizeof(SFX_##tag)-1);
#define SONG(tag) sh_ms("\x02" SONG_##tag,2);
// Songs are listed in src/audio/main.c. Tags are the stem of the original file name.
#define SONG_none "\x00"
#define SONG_across_the_scrubby_moors "\x01"
#define SONG_circus_of_the_night "\x02"
#define SONG_rinky_dink "\x03"
// Sounds, you provide the entire message: 0x01,noteida 0..63,noteidz 0..63,level 0..31,duration 16ms
#define SFX_uimotion "\x01\x20\x30\x08\x05"
#define SFX_typewriter "\x01\x18\x18\x04\x01"
#define SFX_getkey "\x01\x30\x3c\x0c\x10"
#define SFX_unlock "\x01\x2c\x38\x0c\x10"

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

/* Enter MODE_DIALOGUE. We line-break the given text and store it in (g.dlogtext).
 * Source image is two frames of 15x17 in graphics.
 */
void begin_dialogue(const char *src,int srcc,int srcx,int srcy,uint32_t bg,uint32_t fg);
void dialogue_update(double elapsed);
void dialogue_render();

void gameover_begin();
void gameover_update(double elapsed);
void gameover_render();

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
