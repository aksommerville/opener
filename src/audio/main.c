/* audio/main.c
 *
 * Message format. Each message is identified by its leading byte.
 *   0x01 NOTE: [0x01,noteida 0..63,noteidz 0..63,level 0..31,duration 16ms]
 *   0x02 SONG: [0x02,songid]
 */

#include "shovel/shovel.h"
#include "opt/synmin/synmin.h"

extern const int across_the_scrubby_moors_len;
extern const unsigned char across_the_scrubby_moors[];

static float buffer[1024];

//XXX
#define MAVG_LEN 4
static float mavg=0.0;
static float mavgv[MAVG_LEN]={0};
static int mavgp=0;
 
int sha_init(int rate,int chanc) {
  if (synmin_init(rate,chanc)<0) return -1;
  sh_spcm(0,buffer,sizeof(buffer)/sizeof(buffer[0]));
  return 0;
}
 
void sha_update(int framec) {
  unsigned char msg[256];
  int msgc;
  while ((msgc=sh_mr(msg,sizeof(msg)))>0) {
    switch (msg[0]) {
      case 0x01: if (msgc>=5) {
          synmin_note(msg[1],msg[2],msg[3],msg[4]);
        } break;
      case 0x02: if (msgc>=2) {
          const void *src=0;
          int srcc=0;
          switch (msg[1]) {
            #define _(id,tag) case id: src=tag; srcc=tag##_len; break;
            _(1,across_the_scrubby_moors)
            #undef _
          }
          synmin_song(src,srcc,0,1);
        } break;
    }
  }
  synmin_update(buffer,framec);
  
  //XXX Just fooling around, try a moving-average filter.
  float *v=buffer;
  int i=framec;
  for (;i-->0;v++) {
    float adj=(*v)/MAVG_LEN;
    mavg-=mavgv[mavgp];
    mavgv[mavgp]=adj;
    mavg+=adj;
    if (++mavgp>=MAVG_LEN) mavgp=0;
    *v=mavg;
  }
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
