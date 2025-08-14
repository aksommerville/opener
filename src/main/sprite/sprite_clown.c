/* sprite_type_clown.c
 */
 
#include "main/game.h"

#define ANIMFRAME sprite->iv[0]
#define DY sprite->iv[1]
#define ANIMCLOCK sprite->fv[0]
#define FIRECLOCK sprite->fv[1]
#define HURTCLOCK sprite->fv[2]

static int _clown_init(struct sprite *sprite) {
  sprite->tileid=0x20;
  sprite->fg=0xff00ff00;
  return 0;
}

static void _clown_update(struct sprite *sprite,double elapsed) {

  // If FIRECLOCK is set, wind it down, even if we're otherwise noop'd.
  // While set, we don't move or turn or anything else.
  if (FIRECLOCK>0.0) {
    if ((FIRECLOCK-=elapsed)<=0.0) {
      sprite->tileid=0x20;
    }
    return;
  }
  
  // Ditto HURTCLOCK. They can't both be set.
  if (HURTCLOCK>0.0) {
    if ((HURTCLOCK-=elapsed)<=0.0) {
      sprite->tileid=0x20;
    }
    return;
  }

  // Do nothing if the hero is missing. This doesn't happen in real life.
  if (!g.hero) return;
  
  // Noop if we're offscreen.
  if (sprite->x<g.camerax) return;
  if (sprite->y<g.cameray) return;
  if (sprite->x>=g.camerax+FBW) return;
  if (sprite->y>=g.cameray+FBH) return;

  // Face the hero.
  if (g.hero->x<sprite->x) sprite->xform=R1B_XFORM_XREV;
  else if (g.hero->x>sprite->x) sprite->xform=0;
  
  // Animate.
  if ((ANIMCLOCK-=elapsed)<=0.0) {
    ANIMCLOCK+=0.200;
    if (++(ANIMFRAME)>=4) ANIMFRAME=0;
    switch (ANIMFRAME) {
      case 0: case 2: sprite->tileid=0x20; break;
      case 1: sprite->tileid=0x21; break;
      case 3: sprite->tileid=0x22; break;
    }
  }
  
  /* Walk up and down.
   * Since our positions are integers, and their space is very low-rez, we slow it down by skipping frames.
   */
  if (!(g.framec%3)) {
    if (DY<0) {
      sprite->y--;
      if (sprite_rectify(sprite,0,-1)) {
        DY=1;
      }
    } else {
      sprite->y++;
      if (sprite_rectify(sprite,0,1)) {
        DY=-1;
      }
    }
  }
  
  /* Fire a projectile at fixed clock intervals.
   */
  if (!(g.framec%200)) {
    FIRECLOCK=0.500;
    sprite->tileid=0x23;
    struct sprite *fireball=sprite_spawn(&sprite_type_fireball,sprite->x,sprite->y,0);
    if (fireball) {
      fireball->iv[0]=1;
      fireball->xform=sprite->xform;
      SFX(fireball)
    }
  }
}

static void _clown_injure(struct sprite *sprite,struct sprite *assailant) {
  SFX(hurtclown)
  HURTCLOCK=5.0;
  sprite->tileid=0x26;
  FIRECLOCK=0.0;
}

const struct sprite_type sprite_type_clown={
  .name="clown",
  .init=_clown_init,
  .update=_clown_update,
  .injure=_clown_injure,
};
