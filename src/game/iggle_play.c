#include "iggle.h"
#include "sky.h"

/* On completion, it has to actually *stay* complete for so long.
 */
#define COMPLETE_DEBOUNCE_TIME 0.500
#define COMPLETE_FADE_TIME 0.300

struct play {
  int mapid;
  const uint8_t *cellv;
  const uint8_t *cmdv;
  int cmdc;
  double complete_clock;
  double play_time; // For scoring purposes. All levels.
  struct sky sky;
} play={0};

/* End.
 */
 
void play_end() {
}

/* Begin.
 */
 
int play_begin() {
  memset(&play,0,sizeof(play));
  sky_init(&play.sky);
  int mapid=1;
  if (g.advance_to_last_map) mapid=g.last_map_id;
  if (play_load_map(mapid)<0) return -1;
  if (g.enable_music) egg_play_song(RID_song_longhorn,0,1);
  return 0;
}

/* Trivial accessors.
 */
 
const uint8_t *play_get_map() {
  return play.cellv;
}

/* Win level.
 */
 
static void play_win_level() {
  if (play_load_map(play.mapid+1)<0) {
    iggle_check_highscore(play.play_time);
    iggle_set_mode(IGGLE_MODE_FAREWELL);
  }
}

/* Test completion.
 * The pumpkins figure it out, we just poll them.
 */
 
static int play_is_complete() {
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (sprite->defunct) continue;
    if (!sprite->goallable) continue;
    if (!sprite->ongoal) return 0;
  }
  return 1;
}

/* Update.
 */
 
void play_update(double elapsed,int input,int pvinput) {
  sky_update(&play.sky,elapsed);

  if ((input&EGG_BTN_AUX1)&&!(pvinput&EGG_BTN_AUX1)) {
    play_load_map(play.mapid);
  }
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) sprite_hero_button(sprite_any_of_type(&sprite_type_hero),1);
  else if (!(input&EGG_BTN_SOUTH)&&(pvinput&EGG_BTN_SOUTH)) sprite_hero_button(sprite_any_of_type(&sprite_type_hero),0);

  sprites_update(elapsed);
  sprites_drop_defunct();
  
  if (play_is_complete()) {
    if ((play.complete_clock+=elapsed)>=COMPLETE_DEBOUNCE_TIME) {
      play_win_level();
    }
  } else {
    // If it's complete and then not, roll the clock back down rather than resetting it.
    if (play.complete_clock>0.0) {
      play.complete_clock-=elapsed;
    }
    // Play time for tax purposes doesn't count during the ending debounce.
    play.play_time+=elapsed;
  }
}

/* Represent time as tiles.
 * Digits are in row 7: 0..9, colon, dot.
 * Tile zero is blank.
 */
 
static int play_repr_clock(uint8_t *dst,int dsta,double sf) {
  if (dsta<9) return 0;
  int ms=(int)(sf*1000.0);
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  // We could of course add digits to min, but 99 is more minutes than anyone will be playing this game ;)
  if (min>99) { min=sec=99; ms=999; }
  int dstc=0;
  if (min>=10) dst[dstc++]=0x70+min/10;
  dst[dstc++]=0x70+min%10;
  dst[dstc++]=0x7a;
  dst[dstc++]=0x70+sec/10;
  dst[dstc++]=0x70+sec%10;
  dst[dstc++]=0x7b;
  dst[dstc++]=0x70+ms/100;
  dst[dstc++]=0x70+(ms/10)%10;
  dst[dstc++]=0x70+ms%10;
  return dstc;
}

/* Render.
 */
 
void play_render() {

  /* Background.
   */
  sky_render(&play.sky);
  
  /* Terrain.
   */
  graf_draw_decal(&g.graf,g.texid_map,0,0,0,0,FBW,FBH,0);
  
  /* Sprites.
   */
  sprites_render();
  
  /* Clock.
   */
  uint8_t clock[16];
  int clockc=play_repr_clock(clock,sizeof(clock),play.play_time);
  int dstx=FBW-8;
  int dsty=12;
  for (;clockc-->0;dstx-=9) {
    if (!clock[clockc]) continue;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,clock[clockc],0);
  }
  
  /* Fade out.
   */
  if (play.complete_clock>COMPLETE_DEBOUNCE_TIME-COMPLETE_FADE_TIME) {
    int alpha=(int)(255.0*(1.0-(COMPLETE_DEBOUNCE_TIME-play.complete_clock)/COMPLETE_FADE_TIME));
    if (alpha>0) {
      if (alpha>0xff) alpha=0xff;
      graf_draw_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
    }
  }
}

/* Render map bits.
 * Happens only when loading a new map -- cells are immutable.
 */
 
static void play_render_map_bits() {
  egg_draw_clear(g.texid_map,0);
  struct egg_draw_tile vtxv[NS_sys_mapw*NS_sys_maph];
  int vtxc=0;
  const uint8_t *cell=play.cellv;
  int yi=NS_sys_maph,y=NS_sys_tilesize>>1;
  for (;yi-->0;y+=NS_sys_tilesize) {
    int xi=NS_sys_mapw,x=NS_sys_tilesize>>1;
    for (;xi-->0;x+=NS_sys_tilesize,cell++) {
      if (!*cell) continue; // Don't draw zeroes. They show the further background. But the tile is opaque for the editor's sake.
      struct egg_draw_tile *vtx=vtxv+vtxc++;
      vtx->dstx=x;
      vtx->dsty=y;
      vtx->tileid=*cell;
      vtx->xform=0;
    }
  }
  egg_draw_tile(g.texid_map,g.texid_tiles,vtxv,vtxc);
}

/* Spawn sprite.
 */
 
static struct sprite *play_spawn_sprite(uint8_t col,uint8_t row,uint16_t spritetype,uint32_t arg) {
  const struct sprite_type *type=sprite_type_by_id(spritetype);
  if (!type) {
    fprintf(stderr,"spritetype:%d not found\n",spritetype);
    return 0;
  }
  double x=col+0.5;
  double y=row+0.5;
  struct sprite *sprite=sprite_spawn(type,x,y,arg);
  if (!sprite) return 0;
  return sprite;
}

/* Load map.
 */
 
int play_load_map(int mapid) {

  const void *src=0;
  int srcc=iggle_res_get(&src,EGG_TID_map,mapid);
  struct rom_map rmap={0};
  if (rom_map_decode(&rmap,src,srcc)<0) return -1;
  if ((rmap.w!=NS_sys_mapw)||(rmap.h!=NS_sys_maph)) return -1;
  play.cellv=rmap.v;
  play.cmdv=rmap.cmdv;
  play.cmdc=rmap.cmdc;

  // Just being extra cautious since I don't know, this might get called during a sprite update.
  sprites_defunct_all();
  
  struct rom_command_reader reader={.v=play.cmdv,.c=play.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      // CMD_map_image exists but only for the editor's sake -- we're using just one tilesheet.
      case CMD_map_sprite: play_spawn_sprite(cmd.argv[0],cmd.argv[1],(cmd.argv[2]<<8)|cmd.argv[3],(cmd.argv[4]<<24)|(cmd.argv[5]<<16)|(cmd.argv[6]<<8)|cmd.argv[7]); break;
    }
  }
  
  play.mapid=mapid;
  
  play_render_map_bits();
  
  return 0;
}
