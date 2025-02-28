#include "iggle.h"

/* Globals.
 */
 
struct sprite **spritev=0;
int spritec=0;
static int spritea=0;

/* Object lifecycle.
 */

void sprite_del(struct sprite *sprite) {
  if (!sprite) return;
  if (sprite->type->del) sprite->type->del(sprite);
  free(sprite);
}

struct sprite *sprite_new(const struct sprite_type *type,double x,double y,uint32_t arg) {
  if (!type) return 0;
  struct sprite *sprite=calloc(1,type->objlen);
  if (!sprite) return 0;
  sprite->type=type;
  sprite->x=x;
  sprite->y=y;
  sprite->arg=arg;
  if (type->init&&(type->init(sprite)<0)) {
    sprite_del(sprite);
    return 0;
  }
  return sprite;
}

/* Spawn sprite.
 */

struct sprite *sprite_spawn(const struct sprite_type *type,double x,double y,uint32_t arg) {
  struct sprite *sprite=sprite_new(type,x,y,arg);
  if (!sprite) return 0;
  if (spritec>=spritea) {
    int na=spritea+32;
    if (na>INT_MAX/sizeof(void*)) { sprite_del(sprite); return 0; }
    void *nv=realloc(spritev,sizeof(void*)*na);
    if (!nv) { sprite_del(sprite); return 0; }
    spritev=nv;
    spritea=na;
  }
  spritev[spritec++]=sprite;
  return sprite;
}

/* Reap defunct sprites.
 */
 
void sprites_drop_defunct() {
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (!sprite->defunct) continue;
    spritec--;
    memmove(spritev+i,spritev+i+1,sizeof(void*)*(spritec-i));
    sprite_del(sprite);
  }
}

/* Mark all sprites defunct.
 */
 
void sprites_defunct_all() {
  int i=spritec;
  while (i-->0) spritev[i]->defunct=1;
}

/* Find any funct sprite of a given type.
 */

struct sprite *sprite_any_of_type(const struct sprite_type *type) {
  if (!type) return 0;
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (sprite->defunct) continue;
    if (sprite->type!=type) continue;
    return sprite;
  }
  return 0;
}

/* Confirm existence.
 */
 
int sprite_exists(const struct sprite *sprite) {
  if (!sprite) return 0;
  int i=spritec;
  struct sprite **p=spritev;
  for (;i-->0;p++) if (sprite==*p) return 1;
  return 0;
}

/* Update or render all.
 */

void sprites_update(double elapsed) {
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (sprite->defunct) continue;
    if (!sprite->type->update) continue;
    sprite->type->update(sprite,elapsed);
  }
}

void sprites_render() {
  int i=spritec;
  while (i-->0) {
    struct sprite *sprite=spritev[i];
    if (sprite->defunct) continue;
    int x=(int)(sprite->x*NS_sys_tilesize),y=(int)(sprite->y*NS_sys_tilesize);
    if (sprite->type->render) {
      sprite->type->render(sprite,x,y);
    } else if (sprite->tileid) {
      graf_draw_tile(&g.graf,g.texid_tiles,x,y,sprite->tileid,sprite->xform);
    }
  }
}

/* Type list.
 */

const struct sprite_type *sprite_type_by_id(uint16_t spritetype) {
  switch (spritetype) {
    #define _(tag) case NS_spritetype_##tag: return &sprite_type_##tag;
    NS_FOR_EACH_spritetype
    #undef _
  }
  return 0;
}
