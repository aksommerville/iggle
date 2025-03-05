/* sprite_dust.c
 * The little clouds that appear when something hits the ground.
 * One sprite manages both sides.
 * Position us directly on top of the sprite that caused it.
 */
 
#include "game/iggle.h"

#define DUST_TTL 0.400
#define DUST_FRAMEC 4
#define DUST_DX ((NS_sys_tilesize*3)/4)

struct sprite_dust {
  struct sprite hdr;
  double ttl;
};

#define SPRITE ((struct sprite_dust*)sprite)

static int _dust_init(struct sprite *sprite) {
  SPRITE->ttl=DUST_TTL;
  return 0;
}

static void _dust_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->ttl-=elapsed)<=0.0) sprite_kill_soon(sprite);
}

static void _dust_render(struct sprite *sprite,int x,int y) {
  int tileid=(int)((SPRITE->ttl*DUST_FRAMEC)/DUST_TTL);
  if (tileid<0) tileid=0; else if (tileid>=DUST_FRAMEC) tileid=DUST_FRAMEC-1;
  tileid=0x64-tileid;
  graf_draw_tile(&g.graf,g.texid_tiles,x-DUST_DX,y,tileid,0);
  graf_draw_tile(&g.graf,g.texid_tiles,x+DUST_DX,y,tileid,EGG_XFORM_XREV);
}

const struct sprite_type sprite_type_dust={
  .name="dust",
  .objlen=sizeof(struct sprite_dust),
  .init=_dust_init,
  .update=_dust_update,
  .render=_dust_render,
};
