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
  sprite->px=-0.5;
  sprite->py=-0.5;
  sprite->pw=1.0;
  sprite->ph=1.0;
  sprite->tileid=0x20;
  sprite->solid=1;
  return 0;
}

/* Update.
 */
 
static void _pumpkin_update(struct sprite *sprite,double elapsed) {
  // When Iggle carries us, he sets (solid) false. Don't do anything during those times.
  if (sprite->solid) {
    if ((SPRITE->gravity+=GRAVITY_RATE*elapsed)>=GRAVITY_LIMIT) SPRITE->gravity=GRAVITY_LIMIT;
    double gravity=SPRITE->gravity;
    if (gravity<GRAVITY_MIN) gravity=GRAVITY_MIN;
    sprite->y+=gravity*elapsed;
    if (sprite_collide(sprite,0.0,-1.0)) {
      SPRITE->gravity=0.0;
    }
  } else {
    SPRITE->gravity=0.0;
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_pumpkin={
  .name="pumpkin",
  .objlen=sizeof(struct sprite_pumpkin),
  .del=_pumpkin_del,
  .init=_pumpkin_init,
  .update=_pumpkin_update,
};
