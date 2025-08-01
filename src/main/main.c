#include "game.h"

struct g g={0};

int init_images();

/* Quit.
 */
 
void shm_quit(int status) {
}

/* Menu callbacks (TEMP?)
 */
 
static void cb_play(struct ui_menu_option *option) {
  if (reset_game()<0) sh_term(1);
  g.mode=MODE_PLAY;
}
 
static void cb_music(struct ui_menu_option *option) {
  if (g.music_enable) {
    g.music_enable=0;
    option->text="Music: OFF";
    option->textc=10;
    option->w=0;
  } else {
    g.music_enable=1;
    option->text="Music: ON";
    option->textc=9;
    option->w=0;
  }
  //TODO Notify audio thread.
}
 
static void cb_sound(struct ui_menu_option *option) {
  if (g.sound_enable) {
    g.sound_enable=0;
    option->text="Sound: OFF";
    option->textc=10;
    option->w=0;
  } else {
    g.sound_enable=1;
    option->text="Sound: ON";
    option->textc=9;
    option->w=0;
  }
  //TODO Notify audio thread.
}
 
static void cb_quit(struct ui_menu_option *option) {
  sh_term(0);
}

/* Init.
 */

int shm_init() {
  if (init_images()<0) return -1;
  
  g.music_enable=1;//TODO persist, and if initially zero, notify audio thread
  g.sound_enable=1;
  
  g.mode=MODE_HELLO;
  SONG(hello)
  
  g.menu.dsty=22;
  ui_menu_add(&g.menu,"Play",4,0xffffffff,cb_play);
  ui_menu_add(&g.menu,g.music_enable?"Music: ON":"Music: OFF",-1,0xffffff00,cb_music);
  ui_menu_add(&g.menu,g.sound_enable?"Sound: ON":"Music: OFF",-1,0xffffff00,cb_sound);
  ui_menu_add(&g.menu,"Quit",4,0xff0000ff,cb_quit);
  
  g.newsfeed.dsty=29;
  g.newsfeed.src="By AK Sommerville  -  For LowRezJam  -  August 2025  -  ";
  g.newsfeed.srcc=0; while (g.newsfeed.src[g.newsfeed.srcc]) g.newsfeed.srcc++;
  
  return 0;
}

/* Update.
 */

void shm_update(double elapsed) {

  // Gather input, perform global input triggers.
  int input=sh_in(0);
  if (input!=g.pvinput) {
    if ((input&SH_BTN_AUX1)&&!(g.pvinput&SH_BTN_AUX1)) sh_term(0);
    g.pvinput=input;
  }
  
  // Update per mode, and if mode changes, update again.
 _reupdate_:;
  int pvmode=g.mode;
  switch (g.mode) {
    case MODE_HELLO: {
        ui_menu_update(&g.menu,elapsed,input);
        ui_newsfeed_update(&g.newsfeed,elapsed);
      } break;
    case MODE_PLAY: {
        game_update(elapsed,input);
      } break;
    default: g.mode=MODE_HELLO;
  }
  if (pvmode!=g.mode) goto _reupdate_;
  
  // Render.
  switch (g.mode) {
    case MODE_HELLO: {
        r1b_img32_fill_rect(&g.fbimg,0,0,FBW,FBH,0xff000000);
        r1b_img32_blit_img1(&g.fbimg,&g.img_title,
          (FBW>>1)-(g.img_title.w>>1),1,
          0,0,g.img_title.w,g.img_title.h,
          0x00000000,0xff0080ff,0
        );
        ui_menu_render(&g.fbimg,&g.menu);
        ui_newsfeed_render(&g.fbimg,&g.newsfeed);
      } break;
    case MODE_PLAY: {
        game_render();
      } break;
  }
  sh_fb(g.fb,FBW,FBH);
}

/* Annoying clang glue.
 * clang inserts calls to memset and memcpy despite our having told it "nostdlib".
 * Whatever, we can implement them.
 */

#if USE_web
void *memset(void *s, int n, long c) {
  unsigned char *p=s;
  for (;c-->0;p++) *p=n;
  return s;
}
#endif
