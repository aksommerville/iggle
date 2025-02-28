#include "iggle.h"

struct sprite_pumpkin {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_pumpkin*)sprite)

/* Cleanup.
 */
 
static void _pumpkin_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _pumpkin_init(struct sprite *sprite) {
  sprite->tileid=0x20;
  return 0;
}

/* Update.
 */
 
static void _pumpkin_update(struct sprite *sprite,double elapsed) {
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
