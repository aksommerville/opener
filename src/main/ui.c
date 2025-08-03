#include "game.h"

/* menu: Add option.
 */
 
int ui_menu_add(struct ui_menu *menu,const char *text,int textc,uint32_t xbgr,void (*cb)(struct ui_menu_option *option)) {
  if (menu->optionc>=UI_MENU_LIMIT) return -1;
  if (!text) textc=0; else if (textc<0) { textc=0; while (text[textc]) textc++; }
  struct ui_menu_option *option=menu->optionv+menu->optionc++;
  option->text=text;
  option->textc=textc;
  option->xbgr=xbgr;
  option->cb=cb;
  option->w=0;
  return 0;
}

/* menu: Events.
 */
 
static void ui_menu_move(struct ui_menu *menu,int d) {
  if (d<0) menu->transition=-1.0;
  else if (d>0) menu->transition=1.0;
  else return;
  menu->optionp+=d;
  if (menu->optionp>=menu->optionc) menu->optionp=0;
  else if (menu->optionp<0) menu->optionp=menu->optionc-1;
  SFX(uimotion)
}

static void ui_menu_activate(struct ui_menu *menu) {
  if ((menu->optionp<0)||(menu->optionp>=menu->optionc)) return;
  struct ui_menu_option *option=menu->optionv+menu->optionp;
  if (option->cb) option->cb(option);
}

/* menu: Update.
 */
 
#define MENU_BLINK_PERIOD 0.500
#define MENU_BLINK_DUTY   0.350

void ui_menu_update(struct ui_menu *menu,double elapsed) {
  
  const double TRANSITION_RATE=3.000; // hz
  if (menu->transition<0.0) {
    if ((menu->transition+=elapsed*TRANSITION_RATE)>=0.0) {
      menu->transition=0.0;
    }
  } else if (menu->transition>0.0) {
    if ((menu->transition-=elapsed*TRANSITION_RATE)<=0.0) {
      menu->transition=0.0;
    }
  }
  
  if ((menu->blinkclock-=elapsed)<=0.0) {
    menu->blinkclock+=MENU_BLINK_PERIOD;
  }
  
  if (g.input!=g.pvinput) {
    if ((g.input&SH_BTN_LEFT)&&!(g.pvinput&SH_BTN_LEFT)) ui_menu_move(menu,-1);
    if ((g.input&SH_BTN_RIGHT)&&!(g.pvinput&SH_BTN_RIGHT)) ui_menu_move(menu,1);
    if ((g.input&SH_BTN_SOUTH)&&!(g.pvinput&SH_BTN_SOUTH)) ui_menu_activate(menu);
  }
}

/* menu: Calculate option width if we don't have it yet.
 */
 
static inline void ui_menu_option_require_width(struct ui_menu_option *option) {
  if (option->w) return;
  if ((option->w=text_measure(option->text,option->textc))<1) option->w=1;
}

/* menu: Render.
 */
 
void ui_menu_render(struct r1b_img32 *dst,struct ui_menu *menu) {
  r1b_img32_fill_rect(dst,0,menu->dsty,FBW,7,0xff000000);
  
  if ((menu->optionp>=0)&&(menu->optionp<menu->optionc)) {
    struct ui_menu_option *option=menu->optionv+menu->optionp;
    struct ui_menu_option *pv=0;
    int dstx=FBW>>1;
    if (menu->transition<0.0) {
      if (menu->optionp>=menu->optionc-1) pv=menu->optionv;
      else pv=option+1;
    } else if (menu->transition>0.0) {
      if (menu->optionp<=0) pv=menu->optionv+menu->optionc-1;
      else pv=option-1;
    }
    if (pv) {
      dstx+=(int)(FBW*menu->transition);
      int pvdstx=dstx;
      if (menu->transition<0.0) pvdstx+=FBW;
      else pvdstx-=FBW;
      ui_menu_option_require_width(pv);
      pvdstx-=pv->w>>1;
      text_render(dst,pvdstx,menu->dsty+1,pv->text,pv->textc,pv->xbgr);
    }
    ui_menu_option_require_width(option);
    dstx-=option->w>>1;
    text_render(dst,dstx,menu->dsty+1,option->text,option->textc,option->xbgr);
  }
  
  if (menu->blinkclock<=MENU_BLINK_DUTY) {
    text_render(dst,1,menu->dsty+1,"<",1,0xff00ffff);
    text_render(dst,FBW-4,menu->dsty+1,">",1,0xff00ffff);
  }
}

/* newsfeed
 */
 
#define UI_NEWSFEED_SPEED 20.0 /* px/s */

void ui_newsfeed_update(struct ui_newsfeed *nf,double elapsed) {
  nf->clock+=elapsed;
}

void ui_newsfeed_render(struct r1b_img32 *dst,struct ui_newsfeed *nf) {
  r1b_img32_fill_rect(dst,0,nf->dsty,FBW,7,0xff000000);
  if (nf->srcc<1) return;
  if (nf->srcw<1) {
    if ((nf->srcw=text_measure(nf->src,nf->srcc))<1) return;
  }
  int dstx=(int)(FBW-nf->clock*UI_NEWSFEED_SPEED);
  if (dstx<-nf->srcw) dstx%=-nf->srcw;
  for (;dstx<FBW;dstx+=nf->srcw) {
    text_render(dst,dstx,nf->dsty+1,nf->src,nf->srcc,0xff808080);
  }
}
