/* sprite_pumpkin.c
 * arg: u8 tileid, u24 reserved
 * (tileid) zero defaults to 0x50, pumpkin.
 */

#include "iggle.h"

#define GRAVITY_MIN   3.000
#define GRAVITY_RATE  5.000 /* Keep pumpkins' gravity slightly faster than Iggle's. */
#define GRAVITY_LIMIT 9.000 /* This way there's no danger of accidentally re-grabbing it after a bonk. */

struct sprite_pumpkin {
  struct sprite hdr;
  double gravity;
};

#define SPRITE ((struct sprite_pumpkin*)sprite)

/* Cleanup.
 */
 
static void _pumpkin_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _pumpkin_init(struct sprite *sprite) {
  sprite->px=-0.45;
  sprite->py=-0.5;
  sprite->pw=0.9;
  sprite->ph=1.0;
  if (!(sprite->tileid=sprite->arg>>24)) {
    sprite->tileid=0x50;
  }
  sprite->solid=1;
  sprite->grabbable=1;
  sprite->goallable=1;
  return 0;
}

/* Tileid 0x04..0x08, or 1 for the generic goal.
 */
 
static int get_goal_qualifier(int col,int row) {
  int x;
  const uint8_t *cellrow=play_get_map()+row*NS_sys_mapw;
  for (x=col;x-->0;) {
    uint8_t tileid=cellrow[x];
    if ((tileid>=0x04)&&(tileid<=0x08)) return tileid;
    uint8_t physics=g.physics[tileid];
    if (physics!=NS_physics_goal) break;
  }
  for (x=col;x<NS_sys_mapw;x++) {
    uint8_t tileid=cellrow[x];
    if ((tileid>=0x04)&&(tileid<=0x08)) return tileid;
    uint8_t physics=g.physics[tileid];
    if (physics!=NS_physics_goal) break;
  }
  return 1;
}

/* Compare a qualifier to this pumpkin.
 */
 
static int qualifier_agrees_with_pumpkin(uint8_t qualifier,const struct sprite *sprite) {
  switch (qualifier) {
    case 0x01: return 1; // Unqualified: Matches anything.
    case 0x04: switch (sprite->tileid) { // circle
        case 0x50: return 1; // pumpkin
        case 0x51: return 1; // tomato
        case 0x54: return 1; // globe
        case 0x55: return 1; // watermelon
      } break;
    case 0x05: switch (sprite->tileid) { // square
        case 0x52: return 1; // radio
        case 0x53: return 1; // red book
        case 0x56: return 1; // green book
      } break;
    case 0x06: switch (sprite->tileid) { // red
        case 0x51: return 1; // tomato
        case 0x53: return 1; // red book
      } break;
    case 0x07: switch (sprite->tileid) { // green
        case 0x55: return 1; // watermelon
        case 0x56: return 1; // green book
      } break;
    case 0x08: switch (sprite->tileid) { // blue
        case 0x54: return 1; // globe
        case 0x52: return 1; // radio
      } break;
  }
  return 0;
}

/* Check goal.
 */
 
static void pumpkin_check_goal(struct sprite *sprite) {
  sprite->ongoal=0;
  
  int row=(int)(sprite->y+sprite->py+sprite->ph+0.010);
  if ((row>=0)&&(row<NS_sys_maph)) {
    int cola=(int)(sprite->x+sprite->px);
    if (cola<0) cola=0;
    int colz=(int)(sprite->x+sprite->px+sprite->pw-0.000001);
    if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
    const uint8_t *src=play_get_map()+row*NS_sys_mapw+cola;
    int col=cola;
    for (;col<=colz;col++,src++) {
      uint8_t physics=g.physics[*src];
      if (physics==NS_physics_goal) {
        uint8_t qualifier=get_goal_qualifier(col,row);
        if (qualifier_agrees_with_pumpkin(qualifier,sprite)) {
          sprite->ongoal=qualifier;
          return;
        }
      }
    }
  }
  
  double sl=sprite->x+sprite->px;
  double sr=sl+sprite->pw;
  double y=sprite->y+sprite->py+sprite->ph+0.010;
  int i=spritec;
  while (i-->0) {
    const struct sprite *other=spritev[i];
    if (!other->goallable) continue;
    if (!other->ongoal) continue;
    if (!qualifier_agrees_with_pumpkin(other->ongoal,sprite)) continue;
    double ot=other->y+other->py;
    if (y<ot) continue;
    if (y>ot+other->ph) continue;
    double ol=other->x+other->px;
    if (ol>=sr) continue;
    double or=ol+other->pw;
    if (or<=sl) continue;
    sprite->ongoal=other->ongoal;
    return;
  }
}

/* Update.
 */
 
static void _pumpkin_update(struct sprite *sprite,double elapsed) {
  // When Iggle carries us, he sets (solid) false.
  // Not carried, apply gravity, and check goal state only when gravity terminates.
  if (sprite->solid) {
    int wasfalling=(SPRITE->gravity>0.0);
    if ((SPRITE->gravity+=GRAVITY_RATE*elapsed)>=GRAVITY_LIMIT) SPRITE->gravity=GRAVITY_LIMIT;
    double gravity=SPRITE->gravity;
    if (gravity<GRAVITY_MIN) gravity=GRAVITY_MIN;
    sprite->y+=gravity*elapsed;
    if (sprite_collide(sprite,0.0,-1.0)) {
      if (wasfalling) {
        if (g.enable_sound) egg_play_sound(RID_sound_pumpkinfall);
        pumpkin_check_goal(sprite);
      }
      SPRITE->gravity=0.0;
    }
    
  // If we're being carried, check the goal every update. It's not that big a deal.
  } else {
    SPRITE->gravity=0.0;
    pumpkin_check_goal(sprite);
  }
}

/* Render.
 */
 
static void _pumpkin_render(struct sprite *sprite,int x,int y) {
  uint8_t xform=sprite->xform;
  // The two books should not transform, because they have visible text. Not that the text matters.
  if ((sprite->tileid==0x53)||(sprite->tileid==0x56)) xform=0;
  graf_draw_tile(&g.graf,g.texid_tiles,x,y,sprite->tileid,xform);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_pumpkin={
  .name="pumpkin",
  .objlen=sizeof(struct sprite_pumpkin),
  .del=_pumpkin_del,
  .init=_pumpkin_init,
  .update=_pumpkin_update,
  .render=_pumpkin_render,
};
