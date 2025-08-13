#include "game.h"

/* Begin dialogue mode.
 */
 
void begin_dialogue(const char *src,int srcc,int srcx,int srcy,uint32_t bg,uint32_t fg) {
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  g.mode=MODE_DIALOGUE;
  g.dlogsrcx=srcx;
  g.dlogsrcy=srcy;
  g.dlogbg=bg;
  g.dlogfg=fg;
  g.dlogtextp=g.dlogtextc=0;
  const int lineh=6;
  // The first three rows indent with four spaces, ie 16 pixels, to allow room for the portrait.
  int srcp=0,dstx=16,dsty=0,indent=2;
  while (g.dlogtextc<4) g.dlogtext[g.dlogtextc++]=' ';
  #define INDENT if (indent-->0) { \
    if (g.dlogtextc>sizeof(g.dlogtext)-4) break; \
    int i=4; while (i-->0) g.dlogtext[g.dlogtextc++]=' '; \
    dstx=16; \
  }
  while (srcp<srcc) {
  
    // LF: Emit, and begin a new line.
    if (src[srcp]==0x0a) {
      if (g.dlogtextc>=sizeof(g.dlogtext)) break;
      g.dlogtext[g.dlogtextc++]=0x0a;
      srcp++;
      dstx=0;
      dsty+=lineh;
      if (dsty>=FBH) break;
      INDENT
      continue;
    }
    
    // "\x0c(RGB)" or "\x0c()": Preserve verbatim and don't advance position.
    if (src[srcp]==0x0c) {
      while (srcp<srcc) {
        if (g.dlogtextc>=sizeof(g.dlogtext)) break;
        g.dlogtext[g.dlogtextc++]=src[srcp];
        if (src[srcp++]==')') break;
      }
      continue;
    }
    
    // Other whitespace: 4 pixels per, emit, and don't break lines for them.
    if ((unsigned char)src[srcp]<=0x20) {
      if (g.dlogtextc>=sizeof(g.dlogtext)) break;
      g.dlogtext[g.dlogtextc++]=src[srcp];
      srcp++;
      dstx+=4;
      continue;
    }
    
    // Measure the next word, stopping only for whitespace. (which includes control sequences).
    const char *word=src+srcp;
    int wordc=0;
    while ((srcp+wordc<srcc)&&((unsigned char)word[wordc]>0x20)) wordc++;
    
    // If this next word pushes us over the storage limit, get out (don't bother writing what we can).
    if (g.dlogtextc>(int)sizeof(g.dlogtext)-wordc) break;
    
    // If it fits, allowing right up to the final column, commit it.
    int wordlen=text_measure(word,wordc);
    if (dstx<=FBW-wordlen) {
      memcpy(g.dlogtext+g.dlogtextc,word,wordc);
      g.dlogtextc+=wordc;
      srcp+=wordc;
      dstx+=wordlen;
      dstx++;
      continue;
    }
    
    // If we're not at the left margin, insert a newline.
    if (
      ((indent<=0)&&(dstx>0))||
      ((indent>0)&&(dstx>16))
    ) {
      dstx=0;
      dsty+=lineh;
      if (dsty>=FBH) break;
      g.dlogtext[g.dlogtextc++]=0x0a;
      INDENT
      if (g.dlogtextc>(int)sizeof(g.dlogtext)-wordc) break;
    }
    
    // If the word is wider than the framebuffer, chop it off at the last character that fits, but always at least one.
    if (dstx+wordlen>FBW) {
      int tryc=1;
      int trylen=text_measure(word,tryc);
      while ((tryc<wordc)&&(trylen<FBW)) {
        int nextlen=text_measure(word,tryc+1);
        if (nextlen>FBW) break;
        tryc++;
        trylen=nextlen;
      }
      wordc=tryc;
      wordlen=trylen;
    }
    
    // Commit it.
    memcpy(g.dlogtext+g.dlogtextc,word,wordc);
    g.dlogtextc+=wordc;
    srcp+=wordc;
    dstx+=wordlen;
    dstx++; // There's an extra column at the end of each word.
  }
  #undef INDENT
}

/* Update.
 */
 
void dialogue_update(double elapsed) {
  if (g.dlogtextp<g.dlogtextc) {
    if ((g.dlogclock-=elapsed)<=0.0) {
      g.dlogclock+=0.100;
      SFX(typewriter)
      g.dlogtextp++;
      // Consume whitespace and control sequences too; pip only on the real glyphs.
      while (g.dlogtextp<g.dlogtextc) {
        if (g.dlogtext[g.dlogtextp]==0x0c) {
          while ((g.dlogtextp<g.dlogtextc)&&(g.dlogtext[g.dlogtextp]!=')')) g.dlogtextp++;
        } else if ((unsigned char)g.dlogtext[g.dlogtextp]<=0x20) {
          g.dlogtextp++;
        } else {
          break;
        }
      }
    }
  }
  if ((g.dloganimclock-=elapsed)<=0.0) {
    g.dloganimclock+=0.200;
    if (++(g.dlogframe)>=2) g.dlogframe=0;
  }
  if ((g.input&SH_BTN_SOUTH)&&!(g.pvinput&SH_BTN_SOUTH)) {
    if (g.dlogtextp<g.dlogtextc) {
      g.dlogtextp=g.dlogtextc;
    } else {
      g.mode=MODE_PLAY;
    }
  }
}

/* Render.
 */
 
void dialogue_render() {
  r1b_img32_fill_rect(&g.fbimg,0,0,FBW,FBH,0xff000000);
  r1b_img32_blit_img1(&g.fbimg,&g.img_graphics,0,0,g.dlogsrcx+g.dlogframe*15,g.dlogsrcy,15,17,g.dlogbg,g.dlogfg,0);
  text_render(&g.fbimg,0,0,g.dlogtext,g.dlogtextp,0xffffffff);
}
