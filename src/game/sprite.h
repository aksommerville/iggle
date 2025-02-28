#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

struct sprite {
  const struct sprite_type *type;
  double x,y; // Tiles
  double px,py,pw,ph; // Physical bounds relative to (x,y) in tiles.
  uint8_t tileid;
  uint8_t xform;
  uint32_t arg;
  int defunct;
  int solid;
};

struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  void (*update)(struct sprite *sprite,double elapsed);
  void (*render)(struct sprite *sprite,int x,int y); // (x,y) are (sprite->x,y) in framebuffer pixels
};

// READONLY:
extern struct sprite **spritev;
extern int spritec;

/* Avoid. Prefer "spawn" and "kill_soon".
 */
void sprite_del(struct sprite *sprite);
struct sprite *sprite_new(const struct sprite_type *type,double x,double y,uint32_t arg);

struct sprite *sprite_spawn(const struct sprite_type *type,double x,double y,uint32_t arg);

static inline void sprite_kill_soon(struct sprite *sprite) {
  if (sprite) sprite->defunct=1;
}

void sprites_update(double elapsed);
void sprites_render();
void sprites_drop_defunct();
void sprites_defunct_all();

int sprite_exists(const struct sprite *sprite);
struct sprite *sprite_any_of_type(const struct sprite_type *type);

/* Resolve any collisions and return nonzero if changed.
 * I'm doing split-axis motion, so adjust only one axis at a time.
 * (corrx,corry) are a cardinal normal vector indicating which direction we're allowed to correct toward.
 * Against screen edges, we ignore (corr).
 */
int sprite_collide(struct sprite *sprite,double corrx,double corry);

const struct sprite_type *sprite_type_by_id(uint16_t spritetype);

#define _(tag) extern const struct sprite_type sprite_type_##tag;
NS_FOR_EACH_spritetype
#undef _

void sprite_hero_button(struct sprite *sprite,int state);

#endif
