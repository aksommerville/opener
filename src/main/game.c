#include "game.h"

extern const unsigned int palette[];
extern const unsigned char m2[];

/* Reset entire session.
 */
 
int game_reset() {
  //TODO
  return 0;
}

/* Update.
 */
 
//XXX
static double animclock=0.0;
static int animframe=0;
static double turnclock=0.0;
static uint8_t xform=0;
 
void game_update(double elapsed,int input) {
  //TODO
  if ((animclock-=elapsed)<=0.0) {
    animclock+=0.200;
    if (++animframe>=4) animframe=0;
  }
  if ((turnclock-=elapsed)<=0.0) {
    turnclock+=2.000;
    xform^=R1B_XFORM_XREV;
  }
}

/* Render.
 */
 
void game_render() {
  r1b_img32_fill_rect(&g.fbimg,0,0,FBW,FBH,0xff000000);
  
  const unsigned char *map=map_data;
  int dsty=0;
  for (;dsty<FBH;dsty+=TILESIZE) {
    int dstx=0;
    for (;dstx<FBW;dstx+=TILESIZE,map++) {
      int palp=(*map)>>5;
      palp<<=1;
      uint32_t bg=palette[palp];
      uint32_t fg=palette[palp+1];
      uint8_t tileid=(*map)&0x1f;
      int srcx=(tileid&7)*TILESIZE;
      int srcy=(tileid>>3)*TILESIZE;
      r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,bg,fg,0);
    }
  }
  
  const uint32_t animal_colors[4]={
    0xff2fabea, // lion
    0xff2c5b7c, // bear
    0xffcabba7, // elephant
    0xff137bd9, // orangutan
  };
  int srcx=32;
  if (g.pvinput&SH_BTN_SOUTH) srcx+=24;
  else switch (animframe) {
    case 1: srcx+=8; break;
    case 3: srcx+=16; break;
  }
  int dstx=xform?18:38;
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,10,srcx,0,8,8,0,0xffa02050,xform);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,10,64,  0,8,8,0,0xffa0c0f0,xform);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,10,72,  0,8,8,0,0xff1010c0,xform);
  int i=0; for (;i<4;i++) {
    if (xform) dstx+=9; else dstx-=9;
    srcx=32+i*24;
    switch (animframe) {
      case 1: srcx+=8; break;
      case 3: srcx+=16; break;
    }
    r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,10,srcx,8,8,8,0,animal_colors[i],xform);
  }
  //TODO
}
