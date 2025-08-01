#include "tool_internal.h"
#include "opt/png/png.h"

#define FAIL(fmt,...) { \
  if (!path) return -1; \
  fprintf(stderr,"%s: "fmt"\n",path,##__VA_ARGS__); \
  return -2; \
}

/* PNG from PNG.
 */
 
int tool_convert_png(struct sr_encoder *dst,const void *src,int srcc,const char *path) {
  // Easy to imagine adding options in the future like forcing a given pixel format.
  return sr_encode_raw(dst,src,srcc);
}

/* C from font image (16x6 of 4x6, 1-bit).
 */
 
static int tool_convert_c_font_1(struct sr_encoder *dst,const struct png_image *image,int x0,int y0,const char *path,int codepoint) {
  // Read the whole thing one pixel at a time into an i8 buffer. Kind of overkill but will make the analysis easier.
  uint8_t tmp[4*6]={0};
  int yi=0,tmpp=0; for (;yi<6;yi++) {
    int xi=0; for (;xi<4;xi++,tmpp++) {
      const uint8_t *src=((uint8_t*)image->v)+image->stride*(y0+yi)+((x0+xi)>>3);
      uint8_t srcmask=0x80>>((x0+xi)&7);
      if ((*src)&srcmask) tmp[tmpp]=1;
    }
  }
  // Rightmost column is forbidden, it's only there to give the source image breathing room.
  for (yi=0;yi<6;yi++) {
    if (tmp[4*yi+3]) FAIL("Illegal use of right column in font image for 0x%02x.",codepoint)
  }
  // If anything in the bottom row is set, top row must be empty and we set the high bit of output and read from row 1 instead of 0.
  uint16_t packed=0;
  if (memcmp(tmp+5*4,"\0\0\0\0",4)) {
    if (memcmp(tmp,"\0\0\0\0",4)) FAIL("Glyph 0x%02x uses both top and bottom rows -- pick one, we can't do both.",codepoint)
    packed=0x8000;
  }
  const uint8_t *srcrow=tmp;
  if (packed&0x8000) srcrow+=4;
  // Copy (tmp) into (packed), one byte to one bit.
  uint16_t pmask=0x4000;
  for (yi=5;yi-->0;srcrow+=4) {
    const uint8_t *srcp=srcrow;
    int xi=3; for (;xi-->0;srcp++,pmask>>=1) {
      if (*srcp) packed|=pmask;
    }
  }
  // Emit (packed) as a decimal integer, and a comma.
  return sr_encode_fmt(dst,"%d,",packed);
}
 
static int tool_convert_c_font(struct sr_encoder *dst,const struct png_image *image,const char *name,int namec,const char *path) {
  if (!image||(image->depth!=1)||(image->w!=64)||(image->h!=36)) return -1;
  sr_encode_fmt(dst,"const unsigned short %.*s[]={\n",namec,name);
  int rowstart=dst->c;
  int codepoint=0x20;
  for (;codepoint<0x80;codepoint++) {
    int srccol=codepoint&15;
    int srcrow=(codepoint>>4)-2;
    int err=tool_convert_c_font_1(dst,image,srccol*4,srcrow*6,path,codepoint);
    if (err<0) return err;
    if (dst->c-rowstart>=100) { // Break lines at tasteful intervals.
      sr_encode_u8(dst,0x0a);
      rowstart=dst->c;
    }
  }
  sr_encode_fmt(dst,"};\n");
  return sr_encoder_require(dst,0);
}

/* C from PNG.
 */
 
int tool_convert_c_png(struct sr_encoder *dst,const void *src,int srcc,const char *path,const char *dstpath) {

  // Decode PNG.
  struct png_image *image=png_decode(src,srcc,path);
  if (!image) return path?-2:-1;
  
  // Convert if requested. GIMP has this obnoxious habit of saving 1-bit images as 8-bit, so we'll do this a lot.
  int depth=tool_arg_int("depth",5,-1);
  int colortype=tool_arg_int("colortype",9,-1);
  if ((depth>=0)||(colortype>=0)) {
    if (depth<0) depth=image->depth;
    if (colortype<0) colortype=image->colortype;
    struct png_image *converted=png_image_reformat(image,depth,colortype);
    if (!converted) {
      png_image_del(image);
      FAIL("Failed to convert to depth=%d colortype=%d.",depth,colortype)
    }
    png_image_del(image);
    image=converted;
  }
  
  // Determine object name from (dstpath), or (path), or make one up.
  char name[64];
  int namec=tool_c_object_name(name,sizeof(name),path,dstpath);
  if ((namec<1)||(namec>sizeof(name))) {
    memcpy(name,"image",5);
    namec=5;
  }
  
  // opener: If it's our font image, do something slightly different.
  if (path) {
    int pathc=0;
    while (path[pathc]) pathc++;
    if ((pathc>=9)&&!memcmp(path+pathc-9,"/font.png",9)&&(image->depth==1)&&(image->w==64)&&(image->h==36)) { // 100% coincidence that this happens to be our framebuffer size.
      int err=tool_convert_c_font(dst,image,name,namec,path);
      png_image_del(image);
      return err;
    }
  }
  
  // Emit some metadata and begin the payload. Use PNG's natural formatting, why not.
  // stride and pixelsize are knowable from (w,depth,colortype) but include for convenience and validation.
  sr_encode_fmt(dst,"const int %.*s_w=%d;\n",namec,name,image->w);
  sr_encode_fmt(dst,"const int %.*s_h=%d;\n",namec,name,image->h);
  sr_encode_fmt(dst,"const int %.*s_stride=%d;\n",namec,name,image->stride);
  sr_encode_fmt(dst,"const int %.*s_depth=%d;\n",namec,name,image->depth);
  sr_encode_fmt(dst,"const int %.*s_colortype=%d;\n",namec,name,image->colortype);
  sr_encode_fmt(dst,"const int %.*s_pixelsize=%d;\n",namec,name,image->pixelsize);
  sr_encode_fmt(dst,"const unsigned char %.*s_pixels[]={\n",namec,name);
  
  // Emit pixels bytewise as decimal integers, and break lines every 100 bytes or so.
  int linestart=dst->c;
  const uint8_t *p=image->v;
  int c=image->stride*image->h;
  for (;c-->0;p++) {
    sr_encode_fmt(dst,"%d,",*p);
    if (dst->c-linestart>=100) {
      sr_encode_u8(dst,0x0a);
      linestart=dst->c;
    }
  }
  
  sr_encode_raw(dst,"};\n",3);
  png_image_del(image);
  return sr_encoder_require(dst,0);
}

/* Rawimg from PNG.
 */
 
int tool_convert_rawimg_png(struct sr_encoder *dst,const void *src,int srcc,const char *path,const char *dstpath) {

  char pixfmt[16];
  int chsize=0;
  int pixfmtc=tool_split_rawimg_fmt(pixfmt,sizeof(pixfmt),&chsize,dstpath);
  if ((pixfmtc<1)||(pixfmtc>sizeof(pixfmt))) FAIL("Unknown output format.")
  int colortype;
       if ((pixfmtc==1)&&!memcmp(pixfmt,"y",1)) colortype=0;
  else if ((pixfmtc==1)&&!memcmp(pixfmt,"a",1)) colortype=0;
  else if ((pixfmtc==1)&&!memcmp(pixfmt,"i",1)) colortype=3;
  else if ((pixfmtc==2)&&!memcmp(pixfmt,"ya",2)) colortype=4;
  else if ((pixfmtc==3)&&!memcmp(pixfmt,"rgb",3)) colortype=2;
  else if ((pixfmtc==4)&&!memcmp(pixfmt,"rgba",4)) colortype=6;
  else FAIL("Unknown output format.")

  struct png_image *image=png_decode(src,srcc,path);
  if (!image) return path?-2:-1;
  struct png_image *converted=png_image_reformat(image,chsize,colortype);
  png_image_del(image);
  if (!converted) FAIL("Failed to convert. Is the output format PNG-legal?")
  image=converted;

  int err=sr_encode_raw(dst,image->v,image->stride*image->h);
  png_image_del(image);
  return err;
}
