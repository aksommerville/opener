/* audio/main.c
 *
 * Message format. Each message is identified by its leading byte.
 *   0x01 NOTE: [0x01,noteida 0..63,noteidz 0..63,level 0..31,duration 16ms]
 *   0x02 SONG: [0x02,songid]
 *   0x03 ENABLE_SOUND: [0x03,0|1]
 *   0x04 ENABLE_SONG: [0x04,0|1]
 */

#include "shovel/shovel.h"
#include "opt/synmin/synmin.h"

extern const int across_the_scrubby_moors_len;
extern const unsigned char across_the_scrubby_moors[];
extern const int circus_of_the_night_len;
extern const unsigned char circus_of_the_night[];
extern const int rinky_dink_len;
extern const unsigned char rinky_dink[];

static float buffer[1024];
static int songid=0;
static int songrepeat=1;
static int enable_sound=1;
static int enable_song=1;

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

static void play_song(int id) {
  if (id==songid) return;
  const void *src=0;
  int srcc=0;
  songrepeat=1;
  switch (songid=id) {
    #define _(id,tag,r) case id: src=tag; srcc=tag##_len; songrepeat=r; break;
    _(1,across_the_scrubby_moors,1)
    _(2,circus_of_the_night,1)
    _(3,rinky_dink,0)
    #undef _
  }
  if (enable_song) synmin_song(src,srcc,0,songrepeat);
}
 
void sha_update(int framec) {
  unsigned char msg[256];
  int msgc;
  while ((msgc=sh_mr(msg,sizeof(msg)))>0) {
    switch (msg[0]) {
      case 0x01: if (msgc>=5) {
          if (enable_sound) synmin_note(msg[1],msg[2],msg[3],msg[4]);
        } break;
      case 0x02: if (msgc>=2) {
          play_song(msg[1]);
        } break;
      case 0x03: if (msgc>=2) {
          enable_sound=msg[1];
          if (!enable_sound) synmin_silence(); // Cuts off song notes too, but they'll recover.
        } break;
      case 0x04: if (msgc>=2) {
          enable_song=msg[1];
          int pvid=songid;
          if (enable_song) {
            songid=0;
            play_song(pvid);
          } else {
            synmin_silence();
            play_song(0);
            songid=pvid;
          }
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
