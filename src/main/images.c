#include "game.h"

#define IMG_FOR_EACH \
  _(title)
  
#define _(tag) \
  extern const int tag##_w; \
  extern const int tag##_h; \
  extern const int tag##_stride; \
  extern const int tag##_depth; \
  extern const int tag##_colortype; \
  extern const int tag##_pixelsize; \
  extern const unsigned char tag##_pixels[];
IMG_FOR_EACH
#undef _

int init_images() {
  
  g.fbimg.v=g.fb;
  g.fbimg.w=FBW;
  g.fbimg.h=FBH;
  g.fbimg.stridewords=FBW;
  
  #define _(tag) { \
    if (tag##_depth!=1) { \
      sh_log("Illegal pixel size for image "#tag"."); \
      return -1; \
    } \
    g.img_##tag.v=(void*)tag##_pixels; \
    g.img_##tag.w=tag##_w; \
    g.img_##tag.h=tag##_h; \
    g.img_##tag.stride=tag##_stride; \
  }
  IMG_FOR_EACH
  #undef _
  
  return 0;
}
