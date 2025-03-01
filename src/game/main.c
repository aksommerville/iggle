#include "iggle.h"

struct g g={0};

int iggle_set_mode(int mode) {
  if (mode==g.mode) return 0;
  switch (g.mode) {
    case IGGLE_MODE_PLAY: play_end(); break;
    case IGGLE_MODE_HELLO: hello_end(); break;
    case IGGLE_MODE_FAREWELL: farewell_end(); break;
  }
  g.mode=mode;
  switch (g.mode) {
    case IGGLE_MODE_PLAY: return play_begin();
    case IGGLE_MODE_HELLO: return hello_begin();
    case IGGLE_MODE_FAREWELL: return farewell_begin();
  }
  return -1;
}

int iggle_res_get(void *dstpp,int tid,int rid) {
  int lo=0,hi=g.resc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct rom_res *res=g.resv+ck;
         if (tid<res->tid) hi=ck;
    else if (tid>res->tid) lo=ck+1;
    else if (rid<res->rid) hi=ck;
    else if (rid>res->rid) lo=ck+1;
    else {
      *(const void**)dstpp=res->v;
      return res->c;
    }
  }
  return 0;
}

void iggle_load_highscore() {
  g.highscore=999999.999;
  char src[256];
  int srcc=egg_store_get(src,sizeof(src),"highscore",9);
  if ((srcc<1)||(srcc>sizeof(src))) return;
  g.highscore=0.0;
  int srcp=0;
  for (;srcp<srcc;srcp++) {
    int digit=src[srcp]-'0';
    if ((digit<0)||(digit>9)) {
      g.highscore=999999999.0;
      break;
    }
    g.highscore*=10.0;
    g.highscore+=digit;
  }
  g.highscore/=1000.0;
}

void iggle_save_highscore() {
  int ms=(int)(g.highscore*1000.0);
  if (ms>999999999) ms=999999999;
  char tmp[]={
    '0'+(ms/100000000)%10,
    '0'+(ms/ 10000000)%10,
    '0'+(ms/  1000000)%10,
    '0'+(ms/   100000)%10,
    '0'+(ms/    10000)%10,
    '0'+(ms/     1000)%10,
    '0'+(ms/      100)%10,
    '0'+(ms/       10)%10,
    '0'+(ms          )%10,
  };
  egg_store_set("highscore",9,tmp,sizeof(tmp));
}

int iggle_check_highscore(double score) {
  g.recentscore=score;
  if (score>g.highscore) return 0;
  g.highscore=score;
  iggle_save_highscore();
  return 1;
}

static void iggle_load_settings() {
  char v;
  if (egg_store_get(&v,1,"enable_music",12)==1) {
    g.enable_music=(v=='1');
  }
  if (egg_store_get(&v,1,"enable_sound",12)==1) {
    g.enable_sound=(v=='1');
  }
}

void iggle_save_settings() {
  char v;
  v=g.enable_music?'1':'0';
  egg_store_set("enable_music",12,&v,1);
  v=g.enable_sound?'1':'0';
  egg_store_set("enable_sound",12,&v,1);
  // Language is exposed to the user similarly, but that's a whole separate thing.
}

static int iggle_load_tilesheet(const void *v,int c) {
  struct rom_tilesheet_reader reader;
  if (rom_tilesheet_reader_init(&reader,v,c)<0) return -1;
  struct rom_tilesheet_entry entry;
  while (rom_tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid!=NS_tilesheet_physics) continue;
    memcpy(g.physics+entry.tileid,entry.v,entry.c);
  }
  return 0;
}

void egg_client_quit(int status) {
}

int egg_client_init() {

  int fbw=0,fbh=0;
  egg_texture_get_status(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) return -1;
  if ((FBW!=NS_sys_tilesize*NS_sys_mapw)||(FBH!=NS_sys_tilesize*NS_sys_maph)) return -1;
  
  if ((g.romc=egg_get_rom(0,0))<=0) return -1;
  if (!(g.rom=malloc(g.romc))) return -1;
  if (egg_get_rom(g.rom,g.romc)!=g.romc) return -1;
  strings_set_rom(g.rom,g.romc);
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_res *res;
  while (res=rom_reader_next(&reader)) {
    switch (res->tid) {
      case EGG_TID_map:
      case EGG_TID_sprite:
        break;
      case EGG_TID_tilesheet: iggle_load_tilesheet(res->v,res->c); continue;
      default: continue;
    }
    if (g.resc>=g.resa) {
      int na=g.resa+32;
      if (na>INT_MAX/sizeof(struct rom_res)) return -1;
      void *nv=realloc(g.resv,sizeof(struct rom_res)*na);
      if (!nv) return -1;
      g.resv=nv;
      g.resa=na;
    }
    g.resv[g.resc++]=*res;
  }
  
  if (!(g.font=font_new())) return -1;
  if (font_add_image_resource(g.font,0x0020,RID_image_font26_0020)<0) return -1;
  if (!(g.fontsmall=font_new())) return -1;
  if (font_add_image_resource(g.fontsmall,0x0020,RID_image_font9_0020)<0) return -1;
  if (egg_texture_load_image(g.texid_tiles=egg_texture_new(),RID_image_tiles)<0) return -1;
  if (egg_texture_load_raw(g.texid_map=egg_texture_new(),FBW,FBH,0,0,0)<0) return -1;
  
  srand_auto();
  
  g.enable_music=1;
  g.enable_sound=1;
  iggle_load_settings();
  iggle_load_highscore();
  if (iggle_set_mode(IGGLE_MODE_HELLO)<0) return -1;
  
  return 0;
}

void egg_client_update(double elapsed) {
  int pvinput=g.pvinput;
  int input=egg_input_get_one(0);
  if (input&EGG_BTN_AUX3) egg_terminate(0);
  g.pvinput=input;
  switch (g.mode) {
    case IGGLE_MODE_PLAY: play_update(elapsed,input,pvinput); break;
    case IGGLE_MODE_HELLO: hello_update(elapsed,input,pvinput); break;
    case IGGLE_MODE_FAREWELL: farewell_update(elapsed,input,pvinput); break;
    default: egg_terminate(1);
  }
}

void egg_client_render() {
  graf_reset(&g.graf);
  switch (g.mode) {
    case IGGLE_MODE_PLAY: play_render(); break;
    case IGGLE_MODE_HELLO: hello_render(); break;
    case IGGLE_MODE_FAREWELL: farewell_render(); break;
  }
  graf_flush(&g.graf);
}
