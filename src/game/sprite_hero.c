#include "iggle.h"

struct sprite_hero {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_hero*)sprite)

/* Cleanup.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->tileid=0x10;
  return 0;
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
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
  fprintf(stderr,"%s %d\n",__func__,state);
}
