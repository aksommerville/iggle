#include "iggle.h"

#define TEXT_COLOR_INACTIVE 0x102030ff
#define TEXT_COLOR_ACTIVE   0xffff00ff
#define TEXT_COLOR_STATIC   0xa0c0ffff

#define ALPHA_WAIT_TIME 0.750
#define ALPHA_RAMP_TIME 1.000

#define LABEL_LIMIT 8

/* Label ID is also the index in strings:1
 */
#define LABEL_ID_PLAY 7
#define LABEL_ID_LANGUAGE 8
#define LABEL_ID_MUSIC 9
#define LABEL_ID_SOUND 10
#define LABEL_ID_QUIT 11
#define LABEL_ID_BY_AK 14
#define LABEL_ID_HIGHSCORE 4
#define LABEL_ID_LAST_MAP 15

struct hello {
  struct label {
    int texid;
    int x,y,w,h;
    int selectable;
    int id;
  } labelv[LABEL_LIMIT];
  int labelc;
  int focusp; // within (labelv)
  int texid_banner;
  int bannerw,bannerh;
  int texid_seal;
  int sealw,sealh;
  double alpha_wait_clock;
  double alpha_ramp_clock;
} hello={0};

/* End.
 */
 
void hello_end() {
  struct label *label=hello.labelv;
  int i=hello.labelc;
  for (;i-->0;label++) egg_texture_del(label->texid);
  egg_texture_del(hello.texid_banner);
  egg_texture_del(hello.texid_seal);
  memset(&hello,0,sizeof(hello));
}

/* Add label.
 */
 
static struct label *hello_add_label(int id) {
  if (hello.labelc>=LABEL_LIMIT) return 0;
  struct label *label=hello.labelv+hello.labelc++;
  memset(label,0,sizeof(struct label));
  label->id=id;
  return label;
}

static struct label *hello_add_static_label(int id) {
  struct label *label=hello_add_label(id);
  if (!label) return 0;
  label->texid=font_texres_oneline(g.font,1,id,FBW,TEXT_COLOR_INACTIVE);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

static struct label *hello_add_small_label(int id) {
  struct label *label=hello_add_label(id);
  if (!label) return 0;
  label->texid=font_texres_oneline(g.fontsmall,1,id,FBW,TEXT_COLOR_STATIC);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

static struct label *hello_add_string_label(int id,int vid) {
  struct label *label=hello_add_label(id);
  if (!label) return 0;
  char msg[256];
  struct strings_insertion ins={'r',.r={1,vid}};
  int msgc=strings_format(msg,sizeof(msg),1,id,&ins,1);
  if ((msgc<0)||(msgc>sizeof(msg))) msgc=0;
  label->texid=font_tex_oneline(g.font,msg,msgc,FBW,TEXT_COLOR_INACTIVE);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

static void hello_replace_string_label(struct label *label,int vid) {
  egg_texture_del(label->texid);
  char msg[256];
  struct strings_insertion ins={'r',.r={1,vid}};
  int msgc=strings_format(msg,sizeof(msg),1,label->id,&ins,1);
  if ((msgc<0)||(msgc>sizeof(msg))) msgc=0;
  label->texid=font_tex_oneline(g.font,msg,msgc,FBW,TEXT_COLOR_INACTIVE);
  egg_texture_get_status(&label->w,&label->h,label->texid);
}

static struct label *hello_add_time_label(int id,double sf) {
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
  
  struct label *label=hello_add_label(id);
  if (!label) return 0;
  char msg[256];
  struct strings_insertion ins={'s',.s={tmp,tmpc}};
  int msgc=strings_format(msg,sizeof(msg),1,id,&ins,1);
  if ((msgc<0)||(msgc>sizeof(msg))) msgc=0;
  label->texid=font_tex_oneline(g.font,msg,msgc,FBW,TEXT_COLOR_STATIC);
  egg_texture_get_status(&label->w,&label->h,label->texid);
  return label;
}

/* Begin.
 */
 
int hello_begin() {
  memset(&hello,0,sizeof(struct hello));
  
  hello.alpha_wait_clock=ALPHA_WAIT_TIME;
  hello.alpha_ramp_clock=ALPHA_RAMP_TIME;
  
  egg_texture_load_image(hello.texid_banner=egg_texture_new(),RID_image_banner);
  egg_texture_get_status(&hello.bannerw,&hello.bannerh,hello.texid_banner);
  egg_texture_load_image(hello.texid_seal=egg_texture_new(),RID_image_seal);
  egg_texture_get_status(&hello.sealw,&hello.sealh,hello.texid_seal);
  
  struct label *label;
  if (!(label=hello_add_static_label(LABEL_ID_PLAY))) return -1;
  label->selectable=1;
  /*XXX Cheatsy option to play only the last map -- important as I'm developing the maps. Don't leave this enabled. *
  if (!(label=hello_add_static_label(LABEL_ID_LAST_MAP))) return -1;
  label->selectable=1;
  /**/
  /*TODO Enable this if we translate. ...I really don't think there's much need.
  if (!(label=hello_add_string_label(LABEL_ID_LANGUAGE,1))) return -1;
  label->selectable=1;
  /**/
  if (!(label=hello_add_string_label(LABEL_ID_MUSIC,g.enable_music?12:13))) return -1;
  label->selectable=1;
  if (!(label=hello_add_string_label(LABEL_ID_SOUND,g.enable_sound?12:13))) return -1;
  label->selectable=1;
  if (!(label=hello_add_static_label(LABEL_ID_QUIT))) return -1;
  label->selectable=1;
  if (g.highscore<999999.0) {
    if (!(label=hello_add_time_label(LABEL_ID_HIGHSCORE,g.highscore))) return -1;
  }
  if (!(label=hello_add_small_label(LABEL_ID_BY_AK))) return -1;
  
  const int yspacing=6;
  int wmax=0,hsum=0,hsuml=0,i=hello.labelc,selectablec=0,unselectablec=0;
  for (label=hello.labelv;i-->0;label++) {
    if (label->selectable) {
      if (label->w>wmax) wmax=label->w;
      hsum+=label->h;
      selectablec++;
    } else {
      hsuml+=label->h;
      unselectablec++;
    }
  }
  hsum+=(selectablec-1)*yspacing;
  hsuml+=(unselectablec-1)*yspacing;
  int x=FBW-wmax-100; // 100=Allow plenty of expansion room. The labels can change but their left edges won't.
  int y=FBH-20-hsum;
  int yl=FBH-20-hsuml;
  for (label=hello.labelv,i=hello.labelc;i-->0;label++) {
    if (label->selectable) {
      label->x=x;
      label->y=y;
      y+=label->h;
      y+=yspacing;
    } else {
      label->x=(x>>1)-(label->w>>1);
      label->y=yl;
      yl+=label->h;
      yl+=yspacing;
    }
  }
  
  if (g.enable_music) egg_play_song(RID_song_whirligig,0,1);
  return 0;
}

/* Move selection.
 */
 
static void hello_move(int d) {
  hello.alpha_wait_clock=hello.alpha_ramp_clock=0.0;
  int panic=hello.labelc;
  while (panic-->0) {
    hello.focusp+=d;
    if (hello.focusp<0) hello.focusp=hello.labelc-1;
    else if (hello.focusp>=hello.labelc) hello.focusp=0;
    if (hello.labelv[hello.focusp].selectable) {
      egg_play_sound(RID_sound_uimotion);
      return;
    }
  }
}

/* Left or right on selected label.
 */
 
static void hello_adjust(int d) {
  hello.alpha_wait_clock=hello.alpha_ramp_clock=0.0;
  if ((hello.focusp<0)||(hello.focusp>=hello.labelc)) return;
  struct label *label=hello.labelv+hello.focusp;
  switch (label->id) {
    case LABEL_ID_LANGUAGE: {
        if (g.enable_sound) egg_play_sound(RID_sound_uimotion);
        fprintf(stderr,"TODO language %+d\n",d);
      } break;
    case LABEL_ID_MUSIC: {
        if (g.enable_music=g.enable_music?0:1) {
          hello_replace_string_label(label,12);
          egg_play_song(RID_song_whirligig,0,1);
        } else {
          hello_replace_string_label(label,13);
          egg_play_song(0,0,0);
        }
        iggle_save_settings();
      } break;
    case LABEL_ID_SOUND: {
        g.enable_sound=g.enable_sound?0:1;
        iggle_save_settings();
        hello_replace_string_label(label,g.enable_sound?12:13);
      } break;
  }
}

/* Activate selected label.
 */
 
static void hello_activate() {
  if (hello.alpha_ramp_clock>0.0) {
    hello.alpha_wait_clock=hello.alpha_ramp_clock=0.0;
    return;
  }
  if ((hello.focusp<0)||(hello.focusp>=hello.labelc)) return;
  struct label *label=hello.labelv+hello.focusp;
  switch (label->id) {
    case LABEL_ID_PLAY: iggle_set_mode(IGGLE_MODE_PLAY); break;
    case LABEL_ID_LANGUAGE: break;
    case LABEL_ID_MUSIC: hello_adjust(1); break;
    case LABEL_ID_SOUND: hello_adjust(1); break;
    case LABEL_ID_QUIT: egg_terminate(0); break;
    case LABEL_ID_LAST_MAP: g.advance_to_last_map=1; iggle_set_mode(IGGLE_MODE_PLAY); g.advance_to_last_map=0; break;
  }
}

/* Update.
 */
 
void hello_update(double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_LEFT)&&!(pvinput&EGG_BTN_LEFT)) hello_adjust(-1);
  if ((input&EGG_BTN_RIGHT)&&!(pvinput&EGG_BTN_RIGHT)) hello_adjust(1);
  if ((input&EGG_BTN_UP)&&!(pvinput&EGG_BTN_UP)) hello_move(-1);
  if ((input&EGG_BTN_DOWN)&&!(pvinput&EGG_BTN_DOWN)) hello_move(1);
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) hello_activate();
  
  if (hello.alpha_wait_clock>0.0) {
    hello.alpha_wait_clock-=elapsed;
  } else if (hello.alpha_ramp_clock>0.0) {
    hello.alpha_ramp_clock-=elapsed;
  }
}

/* Render.
 */
 
void hello_render() {

  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x204060ff);
  graf_set_tint(&g.graf,0x305070ff);
  graf_draw_decal(&g.graf,hello.texid_seal,(FBW>>1)-(hello.sealw>>1),(FBH>>1)-(hello.sealh>>1),0,0,hello.sealw,hello.sealh,0);
  graf_set_tint(&g.graf,0);
  
  int alpha=0xff;
  if (hello.alpha_wait_clock>0.0) return;
  if (hello.alpha_ramp_clock>0.0) {
    alpha=(int)((1.0-hello.alpha_ramp_clock/ALPHA_RAMP_TIME)*255.0);
    if (alpha<1) return;
    if (alpha>0xff) alpha=0xff;
    graf_set_alpha(&g.graf,alpha);
  }
  graf_draw_decal(&g.graf,hello.texid_banner,(FBW>>1)-(hello.bannerw>>1),0,0,0,hello.bannerw,hello.bannerh,0);
  struct label *label=hello.labelv;
  int i=hello.labelc,hlt=hello.focusp;
  for (;i-->0;label++) {
    if (!hlt--) graf_set_tint(&g.graf,TEXT_COLOR_ACTIVE);
    graf_draw_decal(&g.graf,label->texid,label->x,label->y,0,0,label->w,label->h,0);
    if (hlt==-1) graf_set_tint(&g.graf,0);
  }
  if (alpha!=0xff) graf_set_alpha(&g.graf,0xff);
}
