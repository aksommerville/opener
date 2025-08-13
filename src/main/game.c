#include "game.h"

extern const unsigned int palette[];
extern const unsigned char m2[];

/* Reset entire session.
 */
 
int game_reset() {
  SONG(across_the_scrubby_moors)
  g.spritec=0;
  g.camerax=0;
  g.cameray=0;
  if (!(g.hero=sprite_spawn(&sprite_type_hero,32,18,0))) return -1;
  sprite_spawn(&sprite_type_animal, 24,48,0x00);//XXX TEMP
  sprite_spawn(&sprite_type_animal, 24,16,0x05);//XXX TEMP
  sprite_spawn(&sprite_type_animal, 80,48,0x0a);//XXX TEMP
  sprite_spawn(&sprite_type_animal, 88, 8,0x0f);//XXX TEMP
  
  /* It shouldn't matter, but initialize (heropath) with the current position.
   * Shouldn't matter because you don't start with any animals, by the time you find one, we'll have repopulated the path.
   */
  struct pathpos *pathpos=g.heropath;
  int i=HEROPATH_LIMIT;
  for (;i-->0;pathpos++) {
    pathpos->x=g.hero->x;
    pathpos->y=g.hero->y;
  }
  g.heropathp=0;
  
  return 0;
}

/* Spawn sprite.
 */
 
struct sprite *sprite_spawn(const struct sprite_type *type,int x,int y,uint32_t arg) {
  struct sprite *sprite=0;
  if (g.spritec<SPRITE_LIMIT) {
    sprite=g.spritev+g.spritec++;
  } else {
    struct sprite *q=g.spritev;
    int i=g.spritec;
    for (;i-->0;q++) {
      if (q->defunct) {
        sprite=q;
        break;
      }
    }
    if (!sprite) return 0;
  }
  memset(sprite,0,sizeof(struct sprite));
  sprite->type=type;
  sprite->x=x;
  sprite->y=y;
  sprite->arg=arg;
  sprite->fg=0xffffffff;
  if (type->init&&(type->init(sprite)<0)) {
    sprite->defunct=1;
    return 0;
  }
  return sprite;
}

/* Update.
 */
 
void game_update(double elapsed) {

  /* Locate the key sprites.
   */
  g.hero=0;
  struct sprite *sprite=g.spritev;
  int i=g.spritec;
  for (;i-->0;sprite++) {
    if (sprite->defunct) continue;
    if (sprite->type==&sprite_type_hero) g.hero=sprite;
  }
  
  /* If there's a hero sprite, she updates in advance of the others.
   * Otherwise it depends on render order and gets a little weird.
   */
  if (g.hero&&g.hero->type->update) {
    g.hero->type->update(g.hero,elapsed);
  }

  /* Update the non-hero sprites.
   */
  for (sprite=g.spritev,i=g.spritec;i-->0;sprite++) {
    if (sprite->defunct) continue;
    if (!sprite->type->update) continue;
    if (sprite==g.hero) continue;
    sprite->type->update(sprite,elapsed);
  }
  
  /* Global update logic.
   */
  //TODO termination
  
  /* Select a new camera position, if there's a hero.
   * No hero, whatever, leave it wherever it is.
   */
  if (g.hero) {
    const int xstop=COLC*TILESIZE;
    const int ystop=ROWC*TILESIZE;
    const int xlimit=mapw*TILESIZE-xstop;
    const int ylimit=maph*TILESIZE-ystop;
    int focusx=g.hero->x;
    int focusy=g.hero->y;
    g.cameradstx=focusx-focusx%xstop;
    g.cameradsty=focusy-focusy%ystop;
    if (g.cameradstx<0) g.cameradstx=0; else if (g.cameradstx>xlimit) g.cameradstx=xlimit;
    if (g.cameradsty<0) g.cameradsty=0; else if (g.cameradsty>ylimit) g.cameradsty=ylimit;
    if (g.camerax<g.cameradstx) {
      if ((g.camerax+=PAN_SPEED)>=g.cameradstx) g.camerax=g.cameradstx;
    } else if (g.camerax>g.cameradstx) {
      if ((g.camerax-=PAN_SPEED)<=g.cameradstx) g.camerax=g.cameradstx;
    }
    if (g.cameray<g.cameradsty) {
      if ((g.cameray+=PAN_SPEED)>=g.cameradsty) g.cameray=g.cameradsty;
    } else if (g.cameray>g.cameradsty) {
      if ((g.cameray-=PAN_SPEED)<=g.cameradsty) g.cameray=g.cameradsty;
    }
  }
  
  /* Drop defunct sprites.
   */
  for (i=g.spritec,sprite=g.spritev+g.spritec-1;i-->0;sprite--) {
    if (!sprite->defunct) continue;
    if (sprite->type->del) sprite->type->del(sprite);
    g.spritec--;
    memmove(sprite,sprite+1,sizeof(struct sprite)*(g.spritec-i));
  }
}

/* Render.
 */
 
void game_render() {

  /* One pass of a cocktail sort on the sprite list.
   * That means a pass upward and a pass downward, if the first pass strikes.
   * So we can guarantee that if only one sprite is out of place, we'll definitely complete the sort.
   * Multiple sprites out of place, we might remain missorted for a few frames.
   */
  if (g.spritec>1) {
    int i,more=0;
    for (i=0;i<g.spritec-1;i++) {
      if (sprite_rendercmp(g.spritev+i,g.spritev+i+1)>0) {
        struct sprite tmp=g.spritev[i];
        g.spritev[i]=g.spritev[i+1];
        g.spritev[i+1]=tmp;
        more=1;
      }
    }
    if (more) {
      for (i=g.spritec-1;i-->1;) {
        if (sprite_rendercmp(g.spritev+i,g.spritev+i-1)<0) {
          struct sprite tmp=g.spritev[i];
          g.spritev[i]=g.spritev[i+1];
          g.spritev[i+1]=tmp;
        }
      }
    }
  }
  
  // Map.
  int cola=g.camerax/TILESIZE; if (cola<0) cola=0;
  int rowa=g.cameray/TILESIZE; if (rowa<0) rowa=0;
  int colz=(g.camerax+FBW-1)/TILESIZE; if (colz>=mapw) colz=mapw-1;
  int rowz=(g.cameray+FBH-1)/TILESIZE; if (rowz>=maph) rowz=maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    int mdstx0=cola*TILESIZE-g.camerax;
    int mdsty=rowa*TILESIZE-g.cameray;
    const unsigned char *mrow=map_data+rowa*mapw+cola;
    int row=rowa;
    for (;row<=rowz;row++,mdsty+=TILESIZE,mrow+=mapw) {
      const unsigned char *mp=mrow;
      int col=cola;
      int mdstx=mdstx0;
      for (;col<=colz;col++,mdstx+=TILESIZE,mp++) {
        int palp=(*mp)>>5;
        palp<<=1;
        uint32_t bg=palette[palp];
        uint32_t fg=palette[palp+1];
        uint8_t tileid=(*mp)&0x1f;
        int srcx=(tileid&7)*TILESIZE;
        int srcy=(tileid>>3)*TILESIZE;
        r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,mdstx,mdsty,srcx,srcy,TILESIZE,TILESIZE,bg,fg,0);
      }
    }
  }
  
  // Sprites. There can't be any defunct at this point.
  struct sprite *sprite=g.spritev;
  int i=g.spritec;
  for (;i-->0;sprite++) {
    int dstx=sprite->x-g.camerax;
    int dsty=sprite->y-g.cameray;
    if (sprite->type->render) {
      #define BORDER 10 /* Render if sprite's focus point is this close to the camera. */
      if (dstx<-BORDER) continue;
      if (dsty<-BORDER) continue;
      if (dstx>=FBW+BORDER) continue;
      if (dsty>=FBH+BORDER) continue;
      #undef BORDER
      sprite->type->render(sprite,dstx,dsty);
    } else {
      // Generic sprites are double the TILESIZE.
      if (dstx<-TILESIZE) continue;
      if (dsty<-TILESIZE) continue;
      if (dstx>=FBW+TILESIZE) continue;
      if (dsty>=FBH+TILESIZE) continue;
      int srcx=(sprite->tileid&15)*(TILESIZE<<1);
      int srcy=(sprite->tileid>>4)*(TILESIZE<<1);
      r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,dstx-TILESIZE,dsty-TILESIZE,srcx,srcy,TILESIZE<<1,TILESIZE<<1,sprite->bg,sprite->fg,sprite->xform);
    }
  }
}
