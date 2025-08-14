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
    switch (g.animalc) {
      case 0: {
          begin_dialogue(
            "Liberate the \x0c(ff0)four\x0c() animals and bring them here!\n"
            "You can do it!"
          ,-1,0,24,0x00000000,0xffe592d0);
        } break;
      case 1: {
          begin_dialogue(
            "Find the \x0c(ff0)three\x0c() remaining animals!"
          ,-1,0,24,0x00000000,0xffe592d0);
        } break;
      case 2: {
          begin_dialogue(
            "Find the \x0c(ff0)two\x0c() remaining animals!"
          ,-1,0,24,0x00000000,0xffe592d0);
        } break;
      case 3: {
          begin_dialogue(
            "Just \x0c(ff0)one\x0c() more animal!\n"
            "Go get him!"
          ,-1,0,24,0x00000000,0xffe592d0);
        } break;
      case 4: {
          gameover_begin();
        } break;
    }
  }
  LOOSEFRAMEC=0;
}

/* Render.
 */
 
static void _moonsong_render(struct sprite *sprite,int x,int y) {
  int dstx=x-TILESIZE;
  int dsty=y-TILESIZE;
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,dsty,32,0,8,8,0,0xffe592d0,0);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,dsty,64,0,8,8,0,0xff40c0f0,0);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx,dsty,72,0,8,8,0,0xff802010,0);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_moonsong={
  .name="moonsong",
  .init=_moonsong_init,
  .update=_moonsong_update,
  .collide=_moonsong_collide,
  .render=_moonsong_render,
};
