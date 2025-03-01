#include "iggle.h"

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
} play={0};

/* End.
 */
 
void play_end() {
}

/* Begin.
 */
 
int play_begin() {
  memset(&play,0,sizeof(play));
  if (play_load_map(1)<0) return -1;
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
    iggle_set_mode(IGGLE_MODE_FAREWELL);
  }
}

/* Test completion.
 * Pumpkins must be resting on a goal whose logo agrees with their appearance.
 * They can't rest on each other.
 */
 
static int tile_is_goal(uint8_t tileid,int col,int row,const struct sprite *sprite) {
  // We ought to use a tilesheet for this, but whatever it's a tiny game.
  if (tileid<0x02) return 0;
  if (tileid>0x09) return 0;
  // Some goals have a qualifier tile. If present, the pumpkin must satisfy that criterion.
  int qualifier=0;
  int qx=col-1; for (;qx>=0;qx--) {
    uint8_t qt=play.cellv[row*NS_sys_mapw+qx];
    if ((qt<0x02)||(qt>0x09)) break;
    if ((qt>=0x04)&&(qt<=0x08)) { qualifier=qt; break; }
  }
  if (!qualifier) {
    for (qx=col+1;qx<NS_sys_mapw;qx++) {
      uint8_t qt=play.cellv[row*NS_sys_mapw+qx];
      if ((qt<0x02)||(qt>0x09)) break;
      if ((qt>=0x04)&&(qt<=0x08)) { qualifier=qt; break; }
    }
  }
  return sprite_pumpkin_matches_qualifier(sprite,qualifier);
}
 
static int play_is_complete() {
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (sprite->defunct) continue;
    if (sprite->type!=&sprite_type_pumpkin) continue;
    int row=(int)(sprite->y+sprite->py+sprite->ph+0.125);
    if ((row<0)||(row>=NS_sys_maph)) return 0; // Offscreen, definitely not on the goal.
    int cola=(int)(sprite->x+sprite->px); if (cola<0) cola=0;
    int colz=(int)(sprite->x+sprite->px+sprite->pw-0.000001); if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
    int ok=0;
    const uint8_t *cell=play.cellv+row*NS_sys_mapw+cola;
    int col=cola; for (;col<=colz;col++,cell++) {
      if (tile_is_goal(*cell,col,row,sprite)) {
        ok=1;
        break;
      }
    }
    if (!ok) return 0;
  }
  return 1;
}

/* Update.
 */
 
void play_update(double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_WEST)&&!(pvinput&EGG_BTN_WEST)) { // XXX B to end game
    iggle_set_mode(IGGLE_MODE_FAREWELL);
    return;
  }
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) sprite_hero_button(sprite_any_of_type(&sprite_type_hero),1);
  else if (!(input&EGG_BTN_SOUTH)&&(pvinput&EGG_BTN_SOUTH)) sprite_hero_button(sprite_any_of_type(&sprite_type_hero),0);
  sprites_update(elapsed);
  sprites_drop_defunct();
  
  if (play_is_complete()) {
    if ((play.complete_clock+=elapsed)>=COMPLETE_DEBOUNCE_TIME) {
      play_win_level();
    }
  } else if (play.complete_clock>0.0) {
    // If it's complete and then not, roll the clock back down rather than resetting it.
    play.complete_clock-=elapsed;
  }
}

/* Render.
 */
 
void play_render() {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x80a0c0ff);
  graf_draw_decal(&g.graf,g.texid_map,0,0,0,0,FBW,FBH,0);
  sprites_render();
  
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
