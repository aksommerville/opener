/* sprite_animal.c
 * arg: 28:unused, 3:order, 2:species
 * Order zero means not yet rescued. 1..4 are valid.
 */

#include "main/game.h"

#define ANIMFRAME sprite->iv[0]
#define HEROPATHD sprite->iv[1] /* -HEROPATH_LIMIT..-1 */
#define ORDER sprite->iv[2] /* 0..3, which position in line. Don't move; gameover reads this too. */
#define ANIMCLOCK sprite->fv[0]

#define WALKSPEED 7.5 /* m/s */

/* Init.
 */
 
static int _animal_init(struct sprite *sprite) {
  ORDER=(sprite->arg>>2)&7;
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
  HEROPATHD=-9*(ORDER+1)-1;
  return 0;
}

/* Check for unlocking.
 * Hero exists, and we're not liberated yet.
 */
 
static void animal_check_unlock(struct sprite *sprite) {

  // Must have a key, and animal and hero must be on the same screen.
  if (!g.key) return;
  int myscreenx=sprite->x/(TILESIZE*COLC);
  int heroscreenx=g.hero->x/(TILESIZE*COLC);
  if (myscreenx!=heroscreenx) return;
  int myscreeny=sprite->y/(TILESIZE*ROWC);
  int heroscreeny=g.hero->y/(TILESIZE*ROWC);
  if (myscreeny!=heroscreeny) return;
  
  // Identify the map cell right in front of the hero.
  int col=(g.hero->x+((g.hero->xform&R1B_XFORM_XREV)?-5:5))/TILESIZE;
  if ((col<0)||(col>=mapw)) return;
  int row=g.hero->y/TILESIZE;
  if ((row<0)||(row>=maph)) return;
  uint8_t tileid=map[row*mapw+col];
  if ((tileid==0x18)||(tileid==0x14)) {
    SFX(unlock)
    uint8_t *mrow=map+myscreeny*mapw*ROWC+myscreenx*COLC;
    int yi=ROWC;
    for (;yi-->0;mrow+=mapw) {
      uint8_t *mp=mrow;
      int xi=COLC;
      for (;xi-->0;mp++) {
        if ((*mp==0x18)||(*mp==0x14)) *mp=0x00;
      }
    }
    g.key=0;
    g.animalc++;
    ORDER=g.animalc;
    HEROPATHD=-9*ORDER-1;
  }
}

/* Update.
 */
 
static void _animal_update(struct sprite *sprite,double elapsed) {

  /* When ORDER is zero, we're still in the cage.
   * Face the hero and check for unlocking -- unlock is effected here.
   */
  if (!ORDER) {
    if (g.hero) {
      if (g.hero->x<sprite->x) sprite->xform=R1B_XFORM_XREV;
      else if (g.hero->x>sprite->x) sprite->xform=0;
      animal_check_unlock(sprite);
    }
    return;
  }

  /* Approach some point in (g.heropath), at a position relative to its writehead that we determined at init.
   * HEROPATHD is always negative, and never so negative that it needs more than one bump to become positive.
   * We'll move at a constant speed. Apply a <1-pixel tolerance to avoid jittering when we reach it.
   */
  int walking=0;
  int targetx=sprite->x;
  int targety=sprite->y;
  int hpp=g.heropathp+HEROPATHD;
  if (hpp<0) hpp+=HEROPATH_LIMIT;
  targetx=g.heropath[hpp].x;
  targety=g.heropath[hpp].y;
  int dx=targetx-sprite->x;
  int dy=targety-sprite->y;
  if (dx||dy) {
    sprite->x+=dx;
    sprite->y+=dy;
    walking=1;
    if (dx<0) sprite->xform=R1B_XFORM_XREV;
    else if (dx>0) sprite->xform=0;
    else if (g.hero&&(g.hero->x<sprite->x)) sprite->xform=R1B_XFORM_XREV;
    else if (g.hero&&(g.hero->x>sprite->x)) sprite->xform=0;
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
