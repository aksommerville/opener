#include "game.h"

struct g g={0};

/* Quit.
 */
 
void shm_quit(int status) {
  sh_log("shm_quit");
}

/* Init.
 */

int shm_init() {
  sh_log("shm_init");
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
