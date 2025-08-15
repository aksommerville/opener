#include "game.h"

/* We'll store the settings under the key "settings".
 * Single byte fields:
 *   'm' = Music on (default)
 *   'M' = Music off
 *   's' = Sound on (default)
 *   'S' = Sound off
 * Ignore anything else.
 */

void settings_load() {

  g.music_enable=1;
  g.sound_enable=1;
  
  char tmp[32];
  int tmpc=sh_sg(tmp,sizeof(tmp),"settings",8);
  if ((tmpc>0)&&(tmpc<=sizeof(tmp))) {
    const char *src=tmp;
    int i=tmpc;
    for (;i-->0;src++) {
      switch (*src) {
        case 'm': g.music_enable=1; break;
        case 'M': g.music_enable=0; break;
        case 's': g.sound_enable=1; break;
        case 'S': g.sound_enable=0; break;
      }
    }
  }
}

void settings_save() {
  char tmp[]={
    g.music_enable?'m':'M',
    g.sound_enable?'s':'S',
  };
  sh_ss("settings",8,tmp,sizeof(tmp));
}
