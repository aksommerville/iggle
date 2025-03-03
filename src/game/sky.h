/* sky.h
 * Owned only by "play".
 * Produces the scene's background.
 */
 
#ifndef SKY_H
#define SKY_H

#define STAR_COUNT 256
#define CLOUD_COUNT 40
#define PUFF_COUNT 5

struct sky {
  double dayp; // Time of day in 0..1. Sunrise at 0, sunset at 0.5.... damn we ought to keep time like this in real life.
  double sunt;
  struct star {
    int x,y;
    double ta,tz; // What times are we visible? They should center around midnight (0.75).
    double fa,fz; // Within (ta..tz), the range of time when we are fully opaque.
  } starv[STAR_COUNT];
  struct cloud {
    double dx;
    double ddx;
    double x,y;
    struct puff {
      double x,y; // relative to cloud
      double dx,dy;
      uint8_t tileid;
      uint8_t xform;
    } puffv[PUFF_COUNT];
  } cloudv[CLOUD_COUNT];
};

void sky_init(struct sky *sky);
void sky_update(struct sky *sky,double elapsed);
void sky_render(struct sky *sky);

#endif
