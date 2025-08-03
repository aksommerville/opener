#include "main/game.h"

#define FACEDX sprite->iv[0]
#define ANIMFRAME sprite->iv[1]
#define WALKDIR sprite->iv[2] /* 0x80..0x01, last direction walking. NB read by animals. */
#define ANIMCLOCK sprite->fv[0]

#define WALKSPEED 7.5 /* m/s */

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->tileid=0x04;
  sprite->fg=0xffa02050;
  FACEDX=1;
  return 0;
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {

  /* Transform input state to two signed axes.
   * If she's pressing left or right, face that direction. We don't have up or down graphics.
   */
  int indx=0,indy=0;
  switch (g.input&(SH_BTN_LEFT|SH_BTN_RIGHT)) {
    case SH_BTN_LEFT: FACEDX=indx=-1; break;
    case SH_BTN_RIGHT: FACEDX=indx=1; break;
  }
  switch (g.input&(SH_BTN_UP|SH_BTN_DOWN)) {
    case SH_BTN_UP: indy=-1; break;
    case SH_BTN_DOWN: indy=1; break;
  }
  if (indy<0) {
    if (indx<0) WALKDIR=0x80;
    else if (!indx) WALKDIR=0x40;
    else WALKDIR=0x20;
  } else if (!indy) {
    if (indx<0) WALKDIR=0x10;
    else if (indx>0) WALKDIR=0x08;
  } else {
    if (indx<0) WALKDIR=0x04;
    else if (!indx) WALKDIR=0x02;
    else WALKDIR=0x01;
  }
  
  /* Animate if walking, reset the animation clock if not.
   * And if walking, modify position and rectify immediately.
   */
  if (indx||indy) {
    if ((ANIMCLOCK-=elapsed)<=0.0) {
      ANIMCLOCK+=0.200;
      if (++(ANIMFRAME)>=4) ANIMFRAME=0;
    }
    sprite->x+=WALKSPEED*elapsed*indx;
    sprite->y+=WALKSPEED*elapsed*indy;
    sprite_rectify(sprite,indx,indy);
    // If there was significant motion, say >1mm, update g.heropath.
    int pvp=g.heropathp-1;
    if (pvp<0) pvp+=HEROPATH_LIMIT;
    double dx=sprite->x-g.heropath[pvp].x;
    double dy=sprite->y-g.heropath[pvp].y;
    if ((dx<-0.001)||(dx>0.001)||(dy<-0.001)||(dy>0.001)) {
      g.heropath[g.heropathp].x=sprite->x;
      g.heropath[g.heropathp].y=sprite->y;
      if (++(g.heropathp)>=HEROPATH_LIMIT) g.heropathp=0;
    }
  } else {
    ANIMCLOCK=0.0;
    ANIMFRAME=0;
  }
}

/* Render.
 */
 
static inline void rtile(int dstx,int dsty,uint8_t tileid,uint32_t fg,uint8_t xform) {
  int srcx=(tileid&15)*(TILESIZE<<1);
  int srcy=(tileid>>4)*(TILESIZE<<1);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,dsty,srcx,srcy,TILESIZE<<1,TILESIZE<<1,0,fg,xform);
}
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  uint8_t xform=(FACEDX<0)?R1B_XFORM_XREV:0;
  int dstx=x-TILESIZE;
  int dsty=y-TILESIZE;
  uint8_t bodytile=0x04;
  switch (ANIMFRAME) {
    case 1: bodytile+=1; break;
    case 3: bodytile+=2; break;
  }
  rtile(dstx,dsty,bodytile,0xffa02050,xform); // body and hat
  rtile(dstx,dsty,0x08,0xffa0c0f0,xform); // face
  rtile(dstx,dsty,0x09,0xff1010c0,xform); // hair
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
};
