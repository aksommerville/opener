#include "main/game.h"

/* Tile properties.
 */
 
int tile_is_solid(uint8_t tileid) {
  return tileid; // For now, only a natural zero is passable. Not sure if that will hold forever.
}

/* Adjust sprite position to escape some other rectangle.
 */
 
static void sprite_rectify_1(struct sprite *sprite,int x,int y,int w,int h,int dx,int dy) {
  
  /* Express sprite's bounds as a simple box.
   */
  int sx=sprite->x-TILESIZE;
  int sy=sprite->y-TILESIZE;
  int sw=TILESIZE<<1;
  int sh=TILESIZE<<1;
  
  /* Determine escapement in each of the four cardinal directions (from sprite's perspective).
   * If any of this is <=0, there is in fact no collision and we're done. That will happen -- the map check is not perfect.
   */
  int escn=sy+sh-y; if (escn<=0) return;
  int escs=y+h-sy; if (escs<=0) return;
  int escw=sx+sw-x; if (escw<=0) return;
  int esce=x+w-sx; if (esce<=0) return;
  
  /* If moving cardinally, there is only one corrective option, do it.
   */
  if (!dx) {
    if (dy<0) { sprite->y+=escs; return; }
    if (dy>0) { sprite->y-=escn; return; }
  } else if (!dy) {
    if (dx<0) { sprite->x+=esce; return; }
    if (dx>0) { sprite->x-=escw; return; }
  }
  
  /* If moving diagonally, poison two of the directions, then proceed as if unspecified.
   */
  if (dx<0) escw=999999; else if (dx>0) esce=999999;
  if (dy<0) escn=999999; else if (dy>0) escs=999999;
  
  /* Choose the smallest escapement.
   * Break ties (N, W, E, S) for no particular reason.
   */
  if ((escn<=escw)&&(escn<=esce)&&(escn<=escs)) sprite->y-=escn;
  else if ((escw<=esce)&&(escw<=escs)) sprite->x-=escw;
  else if (esce<=escs) sprite->x+=esce;
  else sprite->y+=escs;
}

/* Rectify position of one sprite.
 */
 
int sprite_rectify(struct sprite *sprite,int dx,int dy) {
  int result=0;
  
  /* Check edges. Sprite size is always (TILESIZE*2), so we space by exactly TILESIZE.
   * These are hard limits. We don't care about the direction of travel, the correcction direction is obvious.
   */
  if (sprite->x<TILESIZE) { sprite->x=TILESIZE; result=1; }
  else if (sprite->x>mapw*TILESIZE-TILESIZE) { sprite->x=mapw*TILESIZE-TILESIZE; result=1; }
  if (sprite->y<TILESIZE) { sprite->y=TILESIZE; result=1; }
  else if (sprite->y>maph*TILESIZE-TILESIZE) { sprite->y=maph*TILESIZE-TILESIZE; result=1; }
  
  /* Check grid.
   * Owing to the edge correction above, the grid bounds we calculate here are guaranteed in bounds.
   * There's usually 4 cells impacted.
   */
  int cola=(sprite->x-TILESIZE)/TILESIZE;
  int colz=(sprite->x+TILESIZE-1)/TILESIZE;
  int rowa=(sprite->y-TILESIZE)/TILESIZE;
  int rowz=(sprite->y+TILESIZE-1)/TILESIZE;
  const uint8_t *maprow=map_data+rowa*mapw+cola;
  int row=rowa;
  for (;row<=rowz;row++,maprow+=mapw) {
    const uint8_t *mapp=maprow;
    int col=cola;
    for (;col<=colz;col++,mapp++) {
      if (tile_is_solid(*mapp)) {
        sprite_rectify_1(sprite,col*TILESIZE,row*TILESIZE,TILESIZE,TILESIZE,dx,dy);
        result=1;
      }
    }
  }
  
  //TODO physics
  return result;
}
