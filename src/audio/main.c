/* audio/main.c
 *
 * Message format. Each message is identified by its leading byte.
 *   0x01 NOTE: [0x01,noteida 0..63,noteidz 0..63,level 0..31,duration 16ms]
 *   0x02 SONG: [0x02,songid]
 */

#include "shovel/shovel.h"
#include "opt/synmin/synmin.h"

static float buffer[1024];
 
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
            //TODO Songs table.
          }
          synmin_song(src,srcc,0,1);
        } break;
    }
  }
  synmin_update(buffer,framec);
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
