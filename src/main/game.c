#include "game.h"

extern const unsigned int palette[];
extern const unsigned char m2[];

/* Reset entire session.
 */
 
int game_reset() {
  //TODO
  SONG(across_the_scrubby_moors)
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
  switch (input&(SH_BTN_LEFT|SH_BTN_RIGHT)) {
    case SH_BTN_LEFT: if (--(g.camerax)<0) g.camerax=0; break;
    case SH_BTN_RIGHT: if (++(g.camerax)>mapw*TILESIZE-FBW) g.camerax=mapw*TILESIZE-FBW; break;
  }
  switch (input&(SH_BTN_UP|SH_BTN_DOWN)) {
    case SH_BTN_UP: if (--(g.cameray)<0) g.cameray=0; break;
    case SH_BTN_DOWN: if (++(g.cameray)>maph*TILESIZE-FBH) g.cameray=maph*TILESIZE-FBH; break;
  }
}

/* Render.
 */
 
void game_render() {
  
  int cola=g.camerax/TILESIZE; if (cola<0) cola=0;
  int rowa=g.cameray/TILESIZE; if (rowa<0) rowa=0;
  int colz=(g.camerax+FBW-1)/TILESIZE; if (colz>=mapw) colz=mapw-1;
  int rowz=(g.cameray+FBH-1)/TILESIZE; if (rowz>=maph) rowz=maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    int mdstx0=cola*TILESIZE-g.camerax;
    int mdsty=rowa*TILESIZE-g.cameray;
    const unsigned char *mrow=map_data+rowa*mapw+cola;
    int row=rowa;
    for (;row<=rowz;row++,mdsty+=TILESIZE,mrow+=mapw) {
      const unsigned char *mp=mrow;
      int col=cola;
      int mdstx=mdstx0;
      for (;col<=colz;col++,mdstx+=TILESIZE,mp++) {
        int palp=(*mp)>>5;
        palp<<=1;
        uint32_t bg=palette[palp];
        uint32_t fg=palette[palp+1];
        uint8_t tileid=(*mp)&0x1f;
        int srcx=(tileid&7)*TILESIZE;
        int srcy=(tileid>>3)*TILESIZE;
        r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,mdstx,mdsty,srcx,srcy,TILESIZE,TILESIZE,bg,fg,0);
      }
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
