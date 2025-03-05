#include "iggle.h"

#define GRAVITY_MIN   3.000
#define GRAVITY_RATE  5.000 /* Keep pumpkins' gravity slightly faster than Iggle's. */
#define GRAVITY_LIMIT 9.000 /* This way there's no danger of accidentally re-grabbing it after a bonk. */

struct sprite_princess {
  struct sprite hdr;
  double gravity;
  double animclock;
};

#define SPRITE ((struct sprite_princess*)sprite)

static int _princess_init(struct sprite *sprite) {
  sprite->tileid=0xb0; // b0/b1=distress, b2=safe
  sprite->px=-0.45;
  sprite->py=-0.5;
  sprite->pw=0.9;
  sprite->ph=1.0;
  sprite->solid=1;
  sprite->grabbable=1;
  sprite->goallable=1;
  return 0;
}
 
static void princess_check_goal(struct sprite *sprite) {
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
        sprite->ongoal=1;
        return;
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

static void _princess_update(struct sprite *sprite,double elapsed) {
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
        princess_check_goal(sprite);
        sprite_spawn(&sprite_type_dust,sprite->x,sprite->y,0);
      }
      SPRITE->gravity=0.0;
    }
    
  // If we're being carried, check the goal every update. It's not that big a deal.
  } else {
    SPRITE->gravity=0.0;
    princess_check_goal(sprite);
  }
  
  if (sprite->ongoal) {
    sprite->tileid=0xb2;
  } else {
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.400;
      if (sprite->tileid==0xb0) sprite->tileid=0xb1;
      else sprite->tileid=0xb0;
    } else if (sprite->tileid==0xb2) {
      sprite->tileid=0xb0;
    }
  }
}

const struct sprite_type sprite_type_princess={
  .name="princess",
  .objlen=sizeof(struct sprite_princess),
  .init=_princess_init,
  .update=_princess_update,
};
