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
  sprite->px=-0.5;
  sprite->py=-0.5;
  sprite->pw=1.0;
  sprite->ph=1.0;
  if (!(sprite->tileid=sprite->arg>>24)) {
    sprite->tileid=0x50;
  }
  sprite->solid=1;
  sprite->grabbable=1;
  sprite->goallable=1;
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

/* Check qualifier.
 */
 
int sprite_pumpkin_matches_qualifier(const struct sprite *sprite,uint8_t tileid) {
  if (!sprite||(sprite->type!=&sprite_type_pumpkin)) return 0;
  switch (tileid) {
    case 0x00: return 1; // Unqualified: Matches anything.
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
