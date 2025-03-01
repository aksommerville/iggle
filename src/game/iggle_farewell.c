#include "iggle.h"

#define LABEL_LIMIT 2

struct farewell {
  struct label {
    int texid;
    int x,y,w,h;
  } labelv[LABEL_LIMIT];
  int labelc;
} farewell={0};

/* End.
 */
 
void farewell_end() {
  struct label *label=farewell.labelv;
  int i=farewell.labelc;
  for (;i-->0;label++) egg_texture_del(label->texid);
  memset(&farewell,0,sizeof(farewell));
}

/* Add label.
 */
 
static struct label *farewell_add_label() {
  if (farewell.labelc>=LABEL_LIMIT) return 0;
  struct label *label=farewell.labelv+farewell.labelc++;
  memset(label,0,sizeof(struct label));
  return label;
}

static struct label *farewell_add_static_label(int strix) {
  struct label *label=farewell_add_label();
  if (!label) return 0;
  label->texid=font_texres_oneline(g.font,1,strix,FBW,0xffffffff);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

static struct label *farewell_add_time_label(int strix,double sf) {
  int ms=(int)(sf*1000.0);
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  if (min>99) { min=sec=99; ms=999; }
  char tmp[9];
  int tmpc=0;
  if (min>=10) tmp[tmpc++]='0'+min/10;
  tmp[tmpc++]='0'+min%10;
  tmp[tmpc++]=':';
  tmp[tmpc++]='0'+sec/10;
  tmp[tmpc++]='0'+sec%10;
  tmp[tmpc++]='.';
  tmp[tmpc++]='0'+ms/100;
  tmp[tmpc++]='0'+(ms/10)%10;
  tmp[tmpc++]='0'+ms%10;
  
  struct label *label=farewell_add_label();
  if (!label) return 0;
  char msg[256];
  struct strings_insertion ins={'s',.s={tmp,tmpc}};
  int msgc=strings_format(msg,sizeof(msg),1,strix,&ins,1);
  if ((msgc<0)||(msgc>sizeof(msg))) msgc=0;
  label->texid=font_tex_oneline(g.font,msg,msgc,FBW,0xffffffff);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

/* Begin.
 */
 
int farewell_begin() {
  memset(&farewell,0,sizeof(farewell));
  egg_play_song(RID_song_whirligig,0,1);
  
  farewell_add_time_label(3,g.recentscore);
  if (g.recentscore<=g.highscore) {
    farewell_add_static_label(5);
  } else {
    farewell_add_time_label(4,g.highscore);
  }
  
  if (farewell.labelc) {
    const int yspacing=8;
    int hsum=0,i=farewell.labelc;
    struct label *label=farewell.labelv;
    for (;i-->0;label++) hsum+=label->h;
    hsum+=(farewell.labelc-1)*yspacing;
    int y=(FBH>>1)-(hsum>>1);
    for (label=farewell.labelv,i=farewell.labelc;i-->0;label++) {
      label->x=(FBW>>1)-(label->w>>1);
      label->y=y;
      y+=label->h;
      y+=yspacing;
    }
  }
  
  return 0;
}

/* Update.
 */
 
void farewell_update(double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    iggle_set_mode(IGGLE_MODE_HELLO);
  }
}

/* Render.
 */
 
void farewell_render() {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x800000ff);
  struct label *label=farewell.labelv;
  int i=farewell.labelc;
  for (;i-->0;label++) {
    graf_draw_decal(&g.graf,label->texid,label->x,label->y,0,0,label->w,label->h,0);
  }
}
