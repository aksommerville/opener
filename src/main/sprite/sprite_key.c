/* sprite_key.c
 */
 
#include "main/game.h"

#define PICKUP_RADIUS 6

static int _key_init(struct sprite *sprite) {
  sprite->tileid=0x0d;
  sprite->fg=0xff00ffff;
  return 0;
}

static void _key_update(struct sprite *sprite,double elapsed) {
  if (g.key) return; // Can only have one key at a time.
  if (!g.hero) return; // Hero can't pick us up if she doesn't exist, we're firm on that.
  int dx=g.hero->x-sprite->x;
  if ((dx<-PICKUP_RADIUS)||(dx>PICKUP_RADIUS)) return;
  int dy=g.hero->y-sprite->y;
  if ((dy<-PICKUP_RADIUS)||(dy>PICKUP_RADIUS)) return;
  SFX(getkey)
  g.key=1;
  sprite->defunct=1;
}

const struct sprite_type sprite_type_key={
  .name="key",
  .init=_key_init,
  .update=_key_update,
};
