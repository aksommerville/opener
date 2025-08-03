/* sprite_animal.c
 * arg: 28:unused, 2:order, 2:species
 */

#include "main/game.h"

#define ANIMFRAME sprite->iv[0]
#define HEROPATHD sprite->iv[1] /* -HEROPATH_LIMIT..-1 */
#define ORDER sprite->iv[2] /* 0..3, which position in line */
#define ANIMCLOCK sprite->fv[0]

#define WALKSPEED 7.5 /* m/s */

/* Init.
 */
 
static int _animal_init(struct sprite *sprite) {
  sprite->layer=1; // We must update after the hero.
  ORDER=(sprite->arg>>2)&3;
  switch (sprite->arg&3) {
    case 0: { // lion
        sprite->tileid=0x14;
        sprite->fg=0xff2fabea;
      } break;
    case 1: { // bear
        sprite->tileid=0x17;
        sprite->fg=0xff2c5b7c;
      } break;
    case 2: { // elephant
        sprite->tileid=0x1a;
        sprite->fg=0xffcabba7;
      } break;
    case 3: { // orangutan
        sprite->tileid=0x1d;
        sprite->fg=0xff137bd9;
      } break;
  }
  HEROPATHD=-20;//TODO Select heropathd based on how many animals.
  return 0;
}

/* Update.
 */
 
static void _animal_update(struct sprite *sprite,double elapsed) {

  /* Approach some point in (g.heropath), at a position relative to its writehead that we determined at init.
   * HEROPATHD is always negative, and never so negative that it needs more than one bump to become positive.
   * We'll move at a constant speed. Apply a <1-pixel tolerance to avoid jittering when we reach it.
   */
  int walking=0;
  double targetx=sprite->x;
  double targety=sprite->y;
  int hpp=g.heropathp+HEROPATHD;
  if (hpp<0) hpp+=HEROPATH_LIMIT;
  targetx=g.heropath[hpp].x;
  targety=g.heropath[hpp].y;
  if (0&&g.hero) {
    int hdx=0,hdy=0;
    switch (g.hero->iv[2]) { // WALKDIR
      case 0x80: hdx=-1; hdy=-1; break;
      case 0x40: hdy=-1; break;
      case 0x20: hdx=1; hdy=-1; break;
      case 0x10: hdx=-1; break;
      case 0x08: hdx=1; break;
      case 0x04: hdx=-1; hdy=1; break;
      case 0x02: hdy=1; break;
      case 0x01: hdx=1; hdy=1; break;
    }
    targetx=g.hero->x-hdx*2.250*(ORDER+1.0);
    targety=g.hero->y-hdy*2.250*(ORDER+1.0);
  }
  double dx=targetx-sprite->x;
  double dy=targety-sprite->y;
  if (dx<0.0) {
    if ((sprite->x-=WALKSPEED*elapsed)<=targetx) sprite->x=targetx;
    walking=1;
    sprite->xform=R1B_XFORM_XREV;
  } else if (dx>0.0) {
    if ((sprite->x+=WALKSPEED*elapsed)>=targetx) sprite->x=targetx;
    walking=1;
    sprite->xform=0;
  } else if (g.hero&&(g.hero->x<sprite->x-0.5)) {
    sprite->xform=R1B_XFORM_XREV;
  } else if (g.hero&&(g.hero->x>sprite->x+0.5)) {
    sprite->xform=0;
  }
  if (dy<0.0) {
    if ((sprite->y-=WALKSPEED*elapsed)<=targety) sprite->y=targety;
    walking=1;
  } else if (dy>0.0) {
    if ((sprite->y+=WALKSPEED*elapsed)>=targety) sprite->y=targety;
    walking=1;
  }

  // Animation.
  if (walking) {
    if ((ANIMCLOCK-=elapsed)<=0.0) {
      ANIMCLOCK+=0.200;
      if (++(ANIMFRAME)>=4) ANIMFRAME=0;
    }
    sprite->tileid=0x14+(sprite->arg&3)*3;
    switch (ANIMFRAME) {
      case 1: sprite->tileid+=1; break;
      case 3: sprite->tileid+=2; break;
    }
  } else {
    ANIMCLOCK=0.0;
    ANIMFRAME=0;
    sprite->tileid=0x14+(sprite->arg&3)*3;
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_animal={
  .name="animal",
  .init=_animal_init,
  .update=_animal_update,
};
