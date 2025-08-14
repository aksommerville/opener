#include "game.h"

#define MARCH_TIME 21.500 /* Should be roughly the song's length. */
#define MARCHER_COUNT 6

static struct marcher {
  int srcx,srcy; // All marchers have two walking frames right of this.
  uint32_t xbgr;
} marcherv[MARCHER_COUNT];

/* Begin.
 */
 
void gameover_begin() {
  g.mode=MODE_GAMEOVER;
  g.gameover_clock=0.0;
  SONG(rinky_dink)
  
  // All marchers are Dot if something goes wrong.
  struct marcher *marcher=marcherv;
  int i=MARCHER_COUNT;
  for (;i-->0;marcher++) {
    marcher->srcx=32;
    marcher->srcy=0;
    marcher->xbgr=0xffa02050;
  }
  marcherv[0].xbgr=0xffe592d0; // Moon.
  struct sprite *sprite=g.spritev;
  for (i=g.spritec;i-->0;sprite++) {
    if (sprite->type!=&sprite_type_animal) continue;
    int order=sprite->iv[2];
    if ((order<1)||(order>4)) continue;
    marcher=marcherv+1+order;
    int tileid=0x14+(sprite->arg&3)*3;
    marcher->srcx=(tileid&15)*8;
    marcher->srcy=(tileid>>4)*8;
    marcher->xbgr=sprite->fg;
  }
}

/* Update.
 */
 
void gameover_update(double elapsed) {
  g.gameover_clock+=elapsed;
  if (g.gameover_clock<5.0) {
    // Input blackout, discard inputs for a few seconds.
  } else {
    if ((g.input&SH_BTN_SOUTH)&&!(g.pvinput&SH_BTN_SOUTH)) {
      g.mode=MODE_HELLO;
      SONG(circus_of_the_night)
    }
  }
}

/* Render.
 */
 
void gameover_render() {

  /* Background and static text.
   * Message widths are precalculated for centering.
   */
  r1b_img32_fill_rect(&g.fbimg,0,0,FBW,FBH,0xff000000);
  text_render(&g.fbimg,14,1,"GAME OVER",9,0xfff0c020);
  text_render(&g.fbimg,2,30,"ANIMALS RESCUED!",16,0xff40c000);
  
  /* Dot, Moon, and the animals march right to left.
   */
  if (g.gameover_clock<MARCH_TIME) {
    const int partyw=54; // 6 members, 8 pixels each, plus 1 pixel between.
    int frame=0;
    switch (((int)(g.gameover_clock*5.000))&3) {
      case 1: frame=1; break;
      case 3: frame=2; break;
    }
    double t=g.gameover_clock/MARCH_TIME;
    int x=FBW-(int)((FBW+partyw)*t);
    int i=MARCHER_COUNT;
    struct marcher *marcher=marcherv;
    for (;i-->0;x+=9,marcher++) {
      int srcx=marcher->srcx+frame*8;
      r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,x,14,srcx,marcher->srcy,8,8,0,marcher->xbgr,R1B_XFORM_XREV);
    }
  }
  
  /* Done marching, and try to cue against the end of song, show a big heart in the middle.
   */
  if (g.gameover_clock>MARCH_TIME) {
    uint32_t xbgr=0xffa0a0ff;
    if (((int)(g.gameover_clock*2.000))&1) xbgr=0xff8070ff;
    r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,25,12,32,16,15,13,0,xbgr,0);
  }
}
