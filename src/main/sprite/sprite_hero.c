#include "main/game.h"

#define FACEDX sprite->iv[0]
#define ANIMFRAME sprite->iv[1]
#define WALKDIR sprite->iv[2] /* 0x80..0x01, last direction walking. NB read by animals. */
#define INJUREDX sprite->iv[3]
#define ANIMCLOCK sprite->fv[0]
#define FIRECLOCK sprite->fv[1]

#define WALKSPEED 7.5 /* m/s */

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->tileid=0x04;
  sprite->fg=0xffa02050;
  sprite->solid=1;
  FACEDX=1;
  return 0;
}

/* Bump 'n Slide, correct the off-axis toward a cell boundary if that direction looks passable.
 * Called after walking fails. Not called for injury motion.
 */
 
static void hero_bumpnslide(struct sprite *sprite,int dx,int dy) {

  int position; // (sprite->x,y), the one we might correct.
  if (dx) position=sprite->y;
  else if (dy) position=sprite->x;
  else return; // why did you call us?
  int mod=position%TILESIZE;
  int cd; // Which direction to correct it (-1,1).
  if (!mod) return;
  if (mod<TILESIZE>>1) cd=-1;
  else if (mod>TILESIZE>>1) cd=1;
  else return;
  
  /* There are two cells that we care about.
   * If they're both passable, bump it.
   */
  int ax,ay,bx,by;
  if (dx<0) {
    ax=bx=(sprite->x-TILESIZE-1)/TILESIZE;
    ay=(sprite->y-TILESIZE)/TILESIZE;
    if (cd>0) ay++;
    by=ay+1;
  } else if (dx>0) {
    ax=bx=(sprite->x+TILESIZE)/TILESIZE;
    ay=(sprite->y-TILESIZE)/TILESIZE;
    if (cd>0) ay++;
    by=ay+1;
  } else if (dy<0) {
    ax=(sprite->x-TILESIZE)/TILESIZE;
    if (cd>0) ax++;
    bx=ax+1;
    ay=by=(sprite->y-TILESIZE-1)/TILESIZE;
  } else if (dy>0) {
    ax=(sprite->x-TILESIZE)/TILESIZE;
    if (cd>0) ax++;
    bx=ax+1;
    ay=by=(sprite->y+TILESIZE)/TILESIZE;
  } else return;
  
  if ((ax<0)||(ax>=mapw)) return;
  if ((ay<0)||(ay>=maph)) return;
  if ((bx<0)||(bx>=mapw)) return;
  if ((by<0)||(by>=maph)) return;
  if (tile_is_solid(map[ay*mapw+ax])) return;
  if (tile_is_solid(map[by*mapw+bx])) return;
  
  int cdx=0,cdy=0;
  if (dx) { sprite->y+=cd; cdx=cd; }
  else { sprite->x+=cd; cdy=cd; }
  sprite_rectify(sprite,cdx,cdy);
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {

  /* If we're being injured, pay that out.
   */
  if (INJUREDX<0) {
    INJUREDX++;
    sprite->x--;
    sprite_rectify(sprite,-1,0);
  } else if (INJUREDX>0) {
    INJUREDX--;
    sprite->x++;
    sprite_rectify(sprite,1,0);
  }
  
  /* Tick down FIRECLOCK.
   */
  if (FIRECLOCK>0.0) {
    if ((FIRECLOCK-=elapsed)<=0.0) {
    }
  }

  /* Transform input state to two signed axes.
   * If she's pressing left or right, face that direction. We don't have up or down graphics.
   */
  int indx=0,indy=0;
  if (!INJUREDX&&(FIRECLOCK<=0.0)) {
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
  }
  
  /* Animate if walking, reset the animation clock if not.
   * And if walking, modify position and rectify immediately.
   */
  if (indx||indy) {
    if ((ANIMCLOCK-=elapsed)<=0.0) {
      ANIMCLOCK+=0.200;
      if (++(ANIMFRAME)>=4) ANIMFRAME=0;
    }
    // Advance and rectify each axis independently. If we did them together, toe-stubbing would be a real and unsolveable problem.
    if (indx) {
      sprite->x+=indx;
      if (sprite_rectify(sprite,indx,0)&&!indy) hero_bumpnslide(sprite,indx,0);
    }
    if (indy) {
      sprite->y+=indy;
      if (sprite_rectify(sprite,0,indy)&&!indx) hero_bumpnslide(sprite,0,indy);
    }
  } else {
    ANIMCLOCK=0.0;
    ANIMFRAME=0;
  }
  
  /* If there was motion, update heropath.
   * This accounts for both fireball damage and regular walking.
   */
  int pvp=g.heropathp-1;
  if (pvp<0) pvp+=HEROPATH_LIMIT;
  int dx=sprite->x-g.heropath[pvp].x;
  int dy=sprite->y-g.heropath[pvp].y;
  if (dx||dy) {
    g.heropath[g.heropathp].x=sprite->x;
    g.heropath[g.heropathp].y=sprite->y;
    if (++(g.heropathp)>=HEROPATH_LIMIT) g.heropathp=0;
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
  sprite->xform=xform; // We don't use this, but other observers do.
  int dstx=x-TILESIZE;
  int dsty=y-TILESIZE;
  uint8_t bodytile=0x04;
  if (FIRECLOCK>0.0) {
    bodytile=0x07;
  } else switch (ANIMFRAME) {
    case 1: bodytile+=1; break;
    case 3: bodytile+=2; break;
  }
  uint32_t overcolor=0;
  if (INJUREDX) overcolor=0xffffffff;
  rtile(dstx,dsty,bodytile,overcolor?overcolor:0xffa02050,xform); // body and hat
  rtile(dstx,dsty,0x08,overcolor?overcolor:0xffa0c0f0,xform); // face
  rtile(dstx,dsty,0x09,overcolor?overcolor:0xff1010c0,xform); // hair
  if (g.key) {
    int kdstx=dstx;
    if (xform) kdstx-=6;
    else kdstx+=6;
    rtile(kdstx,dsty,0x0e,0xff00ffff,xform);
  }
}

/* Injure.
 */
 
static void _hero_injure(struct sprite *sprite,struct sprite *assailant) {
  if (assailant->xform&R1B_XFORM_XREV) {
    INJUREDX=-8;
  } else {
    INJUREDX=8;
  }
  SFX(injure)
}

/* Shoot fireball.
 */
 
void sprite_hero_shoot_fireball(struct sprite *sprite) {
  if (FIRECLOCK>0.0) return;
  if (INJUREDX) return;
  struct sprite *fireball=sprite_spawn(&sprite_type_fireball,sprite->x,sprite->y,0);
  if (!fireball) return;
  fireball->iv[0]=2;
  fireball->xform=sprite->xform;
  FIRECLOCK=0.500;
  SFX(fireball)
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
  .injure=_hero_injure,
};
