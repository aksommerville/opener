/* sprite_fireball.c
 * iv[0]: Allegiance. 0=none, 1=monsters, 2=hero. Which sprites do we hurt?
 * Set (xform) to R1B_XFORM_XREV to travel left. Right by default.
 */
 
#include "main/game.h"

#define ALLEGIANCE sprite->iv[0]
#define ANIMFRAME sprite->iv[1]
#define ANIMCLOCK sprite->fv[0]

static int _fireball_init(struct sprite *sprite) {
  sprite->tileid=0x0a;
  sprite->fg=0xff0000ff;
  return 0;
}

static void _fireball_update(struct sprite *sprite,double elapsed) {

  if ((ANIMCLOCK-=elapsed)<=0.0) {
    ANIMCLOCK+=0.125;
    if (++(ANIMFRAME)>=4) ANIMFRAME=0;
    switch (ANIMFRAME) {
      case 0: sprite->tileid=0x0a; break;
      case 2: sprite->tileid=0x0c; break;
      default: sprite->tileid=0x0b; break;
    }
  }

  int dx=sprite->xform?-1:1;
  sprite->x+=dx;
  if (sprite_rectify(sprite,dx,0)) {
    sprite->defunct=1;
    return;
  }
  
  struct sprite *victim=g.spritev;
  int i=g.spritec;
  for (;i-->0;victim++) {
    if (victim->defunct) continue;
    if (!victim->type->injure) continue;
    int dx=victim->x-sprite->x;
    if ((dx<-4)||(dx>4)) continue;
    int dy=victim->y-sprite->y;
    if ((dy<-4)||(dy>4)) continue;
    switch (ALLEGIANCE) {
      case 1: { // Don't hurt monsters.
          if (victim->type==&sprite_type_clown) continue;
        } break;
      case 2: { // Don't hurt hero.
          if (victim->type==&sprite_type_hero) continue;
        } break;
    }
    victim->type->injure(victim,sprite);
    sprite->defunct=1;
    return;
  }
}

const struct sprite_type sprite_type_fireball={
  .name="fireball",
  .init=_fireball_init,
  .update=_fireball_update,
};
