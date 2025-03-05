#include "iggle.h"

struct sprite_goblin {
  struct sprite hdr;
  double animclock;
  int frame;
};

#define SPRITE ((struct sprite_goblin*)sprite)

static int _goblin_init(struct sprite *sprite) {
  sprite->tileid=0xb0; // b0/b1=distress, b2=safe
  sprite->px=-0.75;
  sprite->py=-1.5;
  sprite->pw=1.5;
  sprite->ph=2.0;
  sprite->solid=1;
  return 0;
}

static void _goblin_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.400;
    if (++(SPRITE->frame)>=2) SPRITE->frame=0;
  }
}

static void _goblin_render(struct sprite *sprite,int x,int y) {
  int srcy=13*NS_sys_tilesize;
  int srcx=SPRITE->frame?(NS_sys_tilesize<<1):0;
  graf_draw_decal(&g.graf,g.texid_tiles,x-NS_sys_tilesize,y-NS_sys_tilesize-(NS_sys_tilesize>>1),srcx,srcy,NS_sys_tilesize<<1,NS_sys_tilesize<<1,0);
}

const struct sprite_type sprite_type_goblin={
  .name="goblin",
  .objlen=sizeof(struct sprite_goblin),
  .init=_goblin_init,
  .update=_goblin_update,
  .render=_goblin_render,
};
