#include "iggle.h"

#define MAX_FLAP_TIME         0.333 /* s */
#define MAX_RISE_TIME         0.125 /* s */
#define GRAVITY_BLACKOUT_TIME 0.100 /* s */
#define MAX_FLAP_POWER       15.000 /* m/s */
#define GRAVITY_RATE          4.000 /* m/s**2 */
#define GRAVITY_LIMIT         8.000 /* m/s */
#define HORZ_MIN              2.000 /* m/s */
#define HORZ_MAX             12.000 /* m/s */
#define HORZ_UP_RATE         10.000 /* m/s**2 */
#define HORZ_DOWN_RATE        5.000 /* m/s**2 */
#define BONK_VELOCITY_LOSS    0.666
#define BONK_GRAVITY          0.000 /* m/s; after a bonk, gravity kicks in immediately. */
#define BONK_BLACKOUT_TIME    0.250 /* s; after a bonk, we ignore input for so long. */
#define PH_NO_PUMPKIN         1.000

struct sprite_hero {
  struct sprite hdr;
  int button;
  int flap;
  int grounded;
  double button_clock;
  double gravity;
  double facedir; // 1,-1
  double velocity;
  double blackout;
  struct sprite *pumpkin;
};

#define SPRITE ((struct sprite_hero*)sprite)

/* Cleanup.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->px=-0.45;
  sprite->pw= 0.90;
  sprite->py=-0.50;
  sprite->ph=PH_NO_PUMPKIN;
  sprite->tileid=0x40;
  sprite->solid=1;
  SPRITE->facedir=1.0f;
  SPRITE->velocity=HORZ_MIN;
  SPRITE->button_clock=999.999;
  return 0;
}

/* Horizontal motion and acceleration.
 */
 
static void hero_update_horizontal(struct sprite *sprite,double elapsed) {

  if (SPRITE->flap) {
    if ((SPRITE->velocity+=HORZ_UP_RATE*elapsed)>=HORZ_MAX) SPRITE->velocity=HORZ_MAX;
  } else {
    if ((SPRITE->velocity-=HORZ_DOWN_RATE*elapsed)<=HORZ_MIN) SPRITE->velocity=HORZ_MIN;
  }
  
  sprite->x+=SPRITE->velocity*elapsed*SPRITE->facedir;
  if (sprite_collide(sprite,-SPRITE->facedir,0.0)) {
    egg_play_sound(RID_sound_horzbonk);
    if ((SPRITE->facedir*=-1.0)<0) sprite->xform=EGG_XFORM_XREV;
    else sprite->xform=0;
  }
}

/* Upward motion, when flapping.
 */
 
static void hero_update_fly(struct sprite *sprite,double elapsed) {
  double power=(MAX_RISE_TIME-SPRITE->button_clock)/MAX_RISE_TIME;
  if (power<=0.0) return;
  power*=MAX_FLAP_POWER;
  sprite->y-=power*elapsed;
  if (sprite_collide(sprite,0.0,1.0)) {
    if (SPRITE->pumpkin) {
      if (sprite_exists(SPRITE->pumpkin)) {
        SPRITE->pumpkin->solid=1;
      }
      egg_play_sound(RID_sound_fullbonk);
      SPRITE->pumpkin=0;
      sprite->ph=PH_NO_PUMPKIN;
    } else {
      egg_play_sound(RID_sound_emptybonk);
    }
    SPRITE->flap=0;
    SPRITE->velocity*=BONK_VELOCITY_LOSS;
    SPRITE->gravity=BONK_GRAVITY;
    SPRITE->button_clock=999.999;
    SPRITE->blackout=BONK_BLACKOUT_TIME;
  }
}

/* Hacky fix for a problem that arises when one pumpkin sits by its toes on the edge of another.
 * Since we're forcing the hero horizontally at grab, need to ensure that there's no other pumpkin colliding with him now.
 */
 
static void cheat_away_nearby_pumpkins(struct sprite *sprite) {
  const double ythresh=0.125; // dy should be pretty much exact.
  const double xthresh=1.000;
  const double shuffle_distance=0.500;
  int i=spritec;
  while (i-->0) {
    struct sprite *pumpkin=spritev[i];
    if (!pumpkin->grabbable) continue;
    double dy=pumpkin->y-sprite->y;
    if ((dy<-ythresh)||(dy>ythresh)) continue;
    double dx=pumpkin->x-sprite->x;
    if ((dx<-xthresh)||(dx>xthresh)) continue;
    if (dx>0.0) {
      pumpkin->x+=shuffle_distance;
      sprite_collide(pumpkin,-1.0,0.0);
    } else {
      pumpkin->x-=shuffle_distance;
      sprite_collide(pumpkin,1.0,0.0);
    }
  }
}

/* Is there a pumpkin below me?
 */
 
static struct sprite *hero_check_pumpkin(const struct sprite *sprite) {
  if (SPRITE->blackout>0.0) return 0;
  double x=sprite->x,y=sprite->y+1.0;
  int i=spritec; while (i-->0) {
    struct sprite *pumpkin=spritev[i];
    if (!pumpkin->grabbable) continue;
    double pl=pumpkin->x+pumpkin->px;
    if (x<=pl) continue;
    double pr=pl+pumpkin->pw;
    if (x>=pr) continue;
    double pt=pumpkin->y+pumpkin->py;
    if (y<=pt) continue;
    double pb=pt+pumpkin->ph;
    if (y>=pb) continue;
    return pumpkin;
  }
  return 0;
}

static struct sprite *hero_check_and_apply_pumpkin(struct sprite *sprite) {
  if (SPRITE->pumpkin) return 0;
  if (SPRITE->pumpkin=hero_check_pumpkin(sprite)) {
    egg_play_sound(RID_sound_grab);
    sprite->x=SPRITE->pumpkin->x;
    sprite->ph=PH_NO_PUMPKIN+SPRITE->pumpkin->ph;
    SPRITE->pumpkin->solid=0;
    cheat_away_nearby_pumpkins(sprite);
  }
  return SPRITE->pumpkin;
}

/* Gravity.
 */
 
static void hero_update_fall(struct sprite *sprite,double elapsed) {
  // There's no gravity while flapping, that's a higher level concern.
  // But also, each flap, even impulses, has a guaranteed minimum no-gravity time.
  if (SPRITE->button_clock<GRAVITY_BLACKOUT_TIME) return;
  if ((SPRITE->gravity+=GRAVITY_RATE*elapsed)>GRAVITY_LIMIT) SPRITE->gravity=GRAVITY_LIMIT;
  sprite->y+=SPRITE->gravity*elapsed;
  if (sprite_collide(sprite,0.0,-1.0)) {
    if (SPRITE->pumpkin) {
      egg_play_sound(RID_sound_alight);
    } else if (hero_check_and_apply_pumpkin(sprite)) {
    } else {
      egg_play_sound(RID_sound_alight);
    }
    SPRITE->grounded=1;
  }
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {

  // (button_clock) advances regardless of button's state.
  SPRITE->button_clock+=elapsed;
  
  if (SPRITE->blackout>0.0) {
    if ((SPRITE->blackout-=elapsed)<=0.0) {
      hero_check_and_apply_pumpkin(sprite);
      SPRITE->blackout=0.0;
      SPRITE->grounded=0;
    }
  }
  
  // Drop (flap) if (button_clock) crosses some threshold.
  if (SPRITE->flap) {
    if (SPRITE->button_clock>MAX_FLAP_TIME) {
      SPRITE->flap=0;
    }
  }
  
  /* When (grounded), basically nothing happens.
   * For now I don't expect any outside kinetics, so we don't need to check footing after the first detection, it can't change.
   */
  if (SPRITE->grounded) {
    sprite->tileid=0x40;
    
  /* Not (grounded), we will move horizontally and vertically -- but Up and Down are separate paths.
   */
  } else {
    hero_update_horizontal(sprite,elapsed);
    if (SPRITE->flap) {
      hero_update_fly(sprite,elapsed);
    } else {
      hero_update_fall(sprite,elapsed);
    }
    // If we're carrying a pumpkin, update its position.
    if (sprite_exists(SPRITE->pumpkin)) {
      SPRITE->pumpkin->x=sprite->x;
      SPRITE->pumpkin->y=sprite->y+sprite->py+PH_NO_PUMPKIN-SPRITE->pumpkin->py;
    }
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
};

/* Button state changed. Search for sprite.
 */
 
void sprite_hero_button(struct sprite *sprite,int state) {
  if (!sprite||(sprite->type!=&sprite_type_hero)) return;
  if (SPRITE->button=state) {
    if (SPRITE->blackout<=0.0) {
      egg_play_sound(RID_sound_flap);
      SPRITE->button_clock=0.0;
      SPRITE->gravity=0.0;
      SPRITE->flap=1;
      SPRITE->grounded=0;
    }
  } else {
    SPRITE->flap=0;
  }
}
