#include "game.h"

struct g g={0};

int init_images();

/* Quit.
 */
 
void shm_quit(int status) {
}

/* Init.
 */

int shm_init() {
  if (init_images()<0) return -1;
  return 0;
}

/* Update.
 */

void shm_update(double elapsed) {

  int input=sh_in(0);
  if (input!=g.pvinput) {
    if ((input&SH_BTN_AUX1)&&!(g.pvinput&SH_BTN_AUX1)) sh_term(0);
    g.pvinput=input;
  }
  
  //TODO Update model.
  //TODO Render.
  r1b_img32_fill_rect(&g.fbimg,0,0,FBW,FBH,0xff000000);
  r1b_img32_blit_img1(&g.fbimg,&g.img_title,
    (FBW>>1)-(g.img_title.w>>1),1,
    0,0,g.img_title.w,g.img_title.h,
    0x00000000,0xff0080ff,0
  );
  text_render(&g.fbimg,10,23,"\x0c(ff0)<\x0c()   Play   \x0c(ff0)>\x0c()",-1,0xffffffff);
  text_render(&g.fbimg,1,30,"TODO: Animate",-1,0xff808080);
  sh_fb(g.fb, FBW, FBH);
}

/* Annoying clang glue.
 * clang inserts calls to memset and memcpy despite our having told it "nostdlib".
 * Whatever, we can implement them.
 */

#if USE_web
void *memset(void *s, int n, long c) {
  unsigned char *p=s;
  for (;c-->0;p++) *p=n;
  return s;
}
#endif
