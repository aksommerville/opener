/* sprite_moonsong.c
 */

#include "main/game.h"

#define LOOSEFRAMEC sprite->iv[0] /* How many frames without a collision? */

/* Init.
 */
 
static int _moonsong_init(struct sprite *sprite) {
  sprite->tileid=0x04;
  sprite->fg=0xffe592d0;
  sprite->solid=1;
  return 0;
}

/* Update.
 */
 
static void _moonsong_update(struct sprite *sprite,double elapsed) {
  LOOSEFRAMEC++;
}

/* Collide with hero.
 */
 
static void _moonsong_collide(struct sprite *sprite) {
  if (LOOSEFRAMEC>=4) {
    fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,__func__);
    //TODO Dialogue.
  }
  LOOSEFRAMEC=0;
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_moonsong={
  .name="moonsong",
  .init=_moonsong_init,
  .update=_moonsong_update,
  .collide=_moonsong_collide,
};
