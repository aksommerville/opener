/* ui.h
 * Self-contained widgets for building a UI.
 */
 
#ifndef UI_H
#define UI_H

/* ui_menu.
 * Full width, 7 pixels high, blacks out its background.
 * Prompts the user to pick one from a list of options.
 * Animation, input.
 */
 
#define UI_MENU_LIMIT 8

struct ui_menu {
  int dsty; // 7 pixels high
  struct ui_menu_option {
    const char *text; // WEAK, borrowed from owner.
    int textc;
    int w; // Output width. If you change (text), zero this.
    uint32_t xbgr;
    void (*cb)(struct ui_menu_option *option);
  } optionv[UI_MENU_LIMIT];
  int optionc;
  int optionp;
  double blinkclock;
  double transition; // -1..0..1 = leftward..idle..rightward
};

// We borrow (text), you must hold it constant.
int ui_menu_add(struct ui_menu *menu,const char *text,int textc,uint32_t xbgr,void (*cb)(struct ui_menu_option *option));

void ui_menu_update(struct ui_menu *menu,double elapsed);
void ui_menu_render(struct r1b_img32 *dst,struct ui_menu *menu);

/* ui_newsfeed.
 * Full width, 7 pixels high, blacks out its background.
 * Shows a scrolling message like at the bottom of the TV news, arbitrarily long.
 * Animation.
 */
 
struct ui_newsfeed {
  int dsty; // 7 pixels high.
  const char *src; // WEAK, borrowed from owner.
  int srcc;
  int srcw; // Zero if you change (src).
  double clock;
};

void ui_newsfeed_update(struct ui_newsfeed *nf,double elapsed);
void ui_newsfeed_render(struct r1b_img32 *dst,struct ui_newsfeed *nf);

//TODO typewriter

#endif
