/* sprite_type_clown.c
 */
 
#include "main/game.h"

static int _clown_init(struct sprite *sprite) {
  sprite->tileid=0x20;
  sprite->fg=0xff00ff00;
  return 0;
}

static void _clown_update(struct sprite *sprite,double elapsed) {
  //TODO
}

const struct sprite_type sprite_type_clown={
  .name="clown",
  .init=_clown_init,
  .update=_clown_update,
};
