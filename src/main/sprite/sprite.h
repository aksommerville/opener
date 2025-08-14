/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

#define SPRITE_IV_SIZE 8
#define SPRITE_FV_SIZE 8

struct sprite {
  const struct sprite_type *type;
  int x,y; // In world pixels, center of sprite.
  uint8_t tileid,xform;
  uint32_t bg,fg;
  int defunct; // The one and only way that game logic should remove a sprite, is to set this nonzero.
  int layer; // Signed, default zero. Higher layers render above others.
  uint32_t arg;
  int solid;
  int iv[SPRITE_IV_SIZE];
  double fv[SPRITE_FV_SIZE];
};

struct sprite_type {
  const char *name;
  
  /* (del) is not called for sprites deleted in a bulk operation like reset.
   * Only if removed during play.
   */
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  
  /* Sprites with an update hook update every cycle, visible or not.
   */
  void (*update)(struct sprite *sprite,double elapsed);
  
  /* If you're a single tile, use (tileid,xform,fg,bg) instead.
   * When (render) is implemented, we call it with the sprite's position in framebuffer space,
   * if within a sensible range of the camera.
   */
  void (*render)(struct sprite *sprite,int x,int y);
  
  /* Called for non-hero solid sprites when the hero collides against them.
   */
  void (*collide)(struct sprite *sprite);
};

static inline int sprite_rendercmp(const struct sprite *a,const struct sprite *b) {
  if (a->layer<b->layer) return -1;
  if (a->layer>b->layer) return 1;
  if (a->y<b->y) return -1;
  if (a->y>b->y) return 1;
  return 0;
}

/* If (sprite) collides with anything solid, force it into a legal position and return nonzero.
 * (dx,dy) are its direction of travel. If you're able to provide these, they might help us choose the most appropriate position.
 */
int sprite_rectify(struct sprite *sprite,int dx,int dy);

int tile_is_solid(uint8_t tile);

extern const struct sprite_type sprite_type_dummy;
extern const struct sprite_type sprite_type_hero;
extern const struct sprite_type sprite_type_animal;
extern const struct sprite_type sprite_type_moonsong;
extern const struct sprite_type sprite_type_key;
extern const struct sprite_type sprite_type_clown;

#endif
