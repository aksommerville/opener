#include "game.h"

extern const unsigned short font[]; /* 0x20..0x7f */

/* Render one glyph.
 * Returns actual width: 1..3
 */
 
static int text_render_char(struct r1b_img32 *dst,int dstx,int dsty,uint16_t src,uint32_t xbgr) {
  if (!src) return 3; // Blank glyph eg space. Space doesn't technically need to be blank.
  if (src&0x8000) dsty++; // High bit shifts it down by one.
  int dstw=(src&0x1249)?3:(src&0x2492)?2:1;
  if ((dstx>=dst->w)||(dstx<=-3)||(dsty>=dst->h)||(dsty<-5)) return dstw;
  int dstx0=dstx;
  uint32_t *dstrow=dst->v+dsty*dst->stridewords+dstx;
  uint16_t mask0=0x4000;
  int yi=5;
  for (;yi-->0;dsty++,dstrow+=dst->stridewords,mask0>>=3) {
    uint16_t mask=mask0;
    int xi=dstw;
    uint32_t *dstp=dstrow;
    for (dstx=dstx0;xi-->0;dstx++,mask>>=1,dstp++) {
      // We could probably do better than bounds-checking every pixel but meh.
      if ((src&mask)&&(dstx>=0)&&(dsty>=0)&&(dstx<dst->w)&&(dsty<dst->h)) {
        *dstp=xbgr;
      }
    }
  }
  return dstw;
}

/* Render text.
 */
 
int text_render(struct r1b_img32 *dst,int dstx,int dsty,const char *src,int srcc,uint32_t xbgr) {
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int dstx0=dstx;
  uint32_t xbgr0=xbgr;
  int srcp=0;
  while (srcp<srcc) {
    int codepoint=(unsigned char)src[srcp++];
    
    // Newline?
    if (codepoint==0x0a) {
      dstx=dstx0;
      dsty+=6;
      continue;
    }
      
    // Color change?
    if ((codepoint==0x0c)&&(srcp<srcc)&&(src[srcp]=='(')) {
      srcp++;
      int valid=1,r=-1,g=-1,b=-1;
      while (srcp<srcc) {
        if (src[srcp]==')') {
          if (b<0) valid=0;
          srcp++;
          break;
        }
        if (b>=0) {
          valid=0;
          break;
        }
        int digit=src[srcp++];
        if ((digit>='0')&&(digit<='9')) digit-='0';
        else if ((digit>='a')&&(digit<='f')) digit=digit-'a'+10;
        else if ((digit>='A')&&(digit<='F')) digit=digit-'A'+10;
        else { srcp--; valid=0; break; }
        if (r<0) r=digit;
        else if (g<0) g=digit;
        else b=digit;
      }
      if (valid) xbgr=0xff000000|(b<<20)|(b<<16)|(g<<12)|(g<<8)|(r<<4)|r;
      else xbgr=xbgr0;
      continue;
    }
    
    // OOB?
    if ((codepoint<0x20)||(codepoint>0x7f)) {
      dstx+=4;
      continue;
    }
    
    // Glyph.
    dstx+=text_render_char(dst,dstx,dsty,font[codepoint-0x20],xbgr);
    dstx++;
  }
  return dstx-dstx0;
}

/* Measure string.
 */
 
int text_measure(const char *src,int srcc) {
  int w=0,srcp=0;
  while (srcp<srcc) {
    int codepoint=(unsigned char)src[srcp++];
    if (codepoint==0x0a) {
      w=0;
      continue;
    }
    if (codepoint==0x0c) {
      while ((srcp<srcc)&&(src[srcp++]!=')')) ; // assume that color escapes are well-formed; incorrect results if not
      continue;
    }
    if ((codepoint<0x20)||(codepoint>0x7f)) {
      w+=4;
      continue;
    }
    uint16_t glyph=font[codepoint-0x20];
    if (!glyph) {
      w+=4;
    } else if (glyph&0x1249) {
      w+=4;
    } else if (glyph&0x2492) {
      w+=3;
    } else {
      w+=2;
    }
  }
  if (w) w--;
  return w;
}
