#include "iggle.h"
#include "sky.h"

#define SKY_DAY_RATE (1/500.0) /* day/sec. I want it possible to finish before sunset, if you push it. 500 = Sunset around 3:15, just beatable. */
#define SUN_SPIN_RATE 2.0 /* rad/sec */
#define WINDD_RANGE 110.0 /* px/s**2 */
#define WIND_LIMIT 10.0 /* px/s */

/* Initialize star.
 */
 
static void star_init(struct star *star) {
  star->x=rand()%FBW;
  star->y=rand()%FBH;
  // Pick a random time range in 1/256 of night.
  int vlen=20+(rand()&0x7f);
  int vp=rand()%(256-vlen);
  star->ta=0.5+(double)vp/512.0;
  star->fa=star->ta+0.005;
  star->tz=0.5+(double)(vp+vlen)/512.0;
  star->fz=star->tz-0.005;
}

/* Initialize cloud.
 * (edge) zero to place randomly anywhere in the sky.
 * Or <0 to create on the left edge, >0 to create on the right edge.
 */
 
static void cloud_init(struct cloud *cloud,int edge) {
  if (edge<0) {
    cloud->x=-5.0;
    cloud->dx=((rand()&0x7fff)*WIND_LIMIT)/32767.0;
    cloud->ddx=((rand()&0x7fff)*WINDD_RANGE)/32767.0; // doesn't strictly need to be positive, but give the cloud a chance of seeing center stage
  } else if (edge>0) {
    cloud->x=FBW+5.0;
    cloud->dx=((rand()&0x7fff)*WIND_LIMIT)/-32767.0;
    cloud->ddx=((rand()&0x7fff)*WINDD_RANGE)/-32767.0;
  } else {
    cloud->x=rand()%FBW;
    cloud->dx=(((rand()&0x7fff)-16384.0)*WIND_LIMIT)/16384.0;
    cloud->ddx=(((rand()&0x7fff)-16384.0)*WINDD_RANGE)/16384.0;
  }
  cloud->y=rand()%FBH;
  struct puff *puff=cloud->puffv;
  int i=PUFF_COUNT;
  for (;i-->0;puff++) {
    puff->x=((rand()&0xff)-0x80)/12.8;
    puff->y=((rand()&0xff)-0x80)/12.8;
    puff->xform=(rand()&1)?EGG_XFORM_XREV:0;
    switch (rand()%6) {
      case 0: puff->tileid=0x86; break;
      case 1: puff->tileid=0x87; break;
      case 2: puff->tileid=0x88; break;
      case 3: puff->tileid=0x96; break;
      case 4: puff->tileid=0x97; break;
      case 5: puff->tileid=0x98; break;
    }
  }
}

/* Init.
 */
 
void sky_init(struct sky *sky) {
  sky->dayp=0.100; // Start a little after sunrise.
  struct star *star=sky->starv;
  int i=STAR_COUNT;
  for (;i-->0;star++) star_init(star);
  struct cloud *cloud=sky->cloudv;
  for (i=CLOUD_COUNT;i-->0;cloud++) cloud_init(cloud,0);
}

/* Update cloud.
 */
 
static void update_cloud(struct sky *sky,struct cloud *cloud,double elapsed) {
  double pvdx=cloud->dx;
  cloud->ddx+=(((rand()&0x7fff)-16384.0)*WINDD_RANGE*elapsed)/16384.0;
  cloud->dx+=cloud->ddx*elapsed;
  if (cloud->dx<-WIND_LIMIT) cloud->dx=-WIND_LIMIT;
  else if (cloud->dx>WIND_LIMIT) cloud->dx=WIND_LIMIT;
  cloud->x+=cloud->dx*elapsed;
  if (cloud->dx<0.0) {
    if (cloud->x<-10.0) cloud_init(cloud,1);
  } else {
    if (cloud->x>FBW+10.0) cloud_init(cloud,-1);
  }
  
  /* Puffs are drawn toward the center but away from each other.
   */
  const double PUFF_CENTRAL_SPEED=4.0;
  const double PUFF_DECOHERE_SPEED=6.0;
  const double PUFF_SPEED_LIMIT=3.0;
  double cx=0.0,cy=0.0;
  struct puff *puff=cloud->puffv;
  int i=PUFF_COUNT;
  for (;i-->0;puff++) {
    cx+=puff->x;
    cy+=puff->y;
  }
  cx/=PUFF_COUNT;
  cy/=PUFF_COUNT;
  for (puff=cloud->puffv,i=PUFF_COUNT;i-->0;puff++) {
  
    if (puff->x>0.0) puff->dx-=PUFF_CENTRAL_SPEED*elapsed;
    else puff->dx+=PUFF_CENTRAL_SPEED*elapsed;
    if (puff->y>0.0) puff->dy-=PUFF_CENTRAL_SPEED*elapsed;
    else puff->dy+=PUFF_CENTRAL_SPEED*elapsed;
  
    double rx=0.0,ry=0.0;
    struct puff *other=cloud->puffv;
    int bi=PUFF_COUNT;
    for (;bi-->0;other++) {
      if (other->x!=puff->x) rx+=1.0/(other->x-puff->x);
      if (other->y!=puff->y) ry+=1.0/(other->y-puff->y);
    }
    double rmag=sqrt(rx*rx+ry*ry);
    if (rmag>0.001) {
      rx/=rmag;
      ry/=rmag;
      puff->dx+=rx*elapsed*PUFF_DECOHERE_SPEED;
      puff->dy+=ry*elapsed*PUFF_DECOHERE_SPEED;
      if (puff->dx<-PUFF_SPEED_LIMIT) puff->dx=-PUFF_SPEED_LIMIT;
      else if (puff->dx>PUFF_SPEED_LIMIT) puff->dx=PUFF_SPEED_LIMIT;
      if (puff->dy<-PUFF_SPEED_LIMIT) puff->dy=-PUFF_SPEED_LIMIT;
      else if (puff->dy>PUFF_SPEED_LIMIT) puff->dy=PUFF_SPEED_LIMIT;
    }
    
    puff->x+=puff->dx*elapsed;
    puff->y+=puff->dy*elapsed;
  }
}

/* Update.
 */
 
void sky_update(struct sky *sky,double elapsed) {

  sky->dayp+=elapsed*SKY_DAY_RATE;
  if (sky->dayp>=1.0) sky->dayp-=1.0;
  
  sky->sunt+=elapsed*SUN_SPIN_RATE;
  if (sky->sunt>=M_PI) sky->sunt-=M_PI*2.0;
  
  struct cloud *cloud=sky->cloudv;
  int i=CLOUD_COUNT;
  for (;i-->0;cloud++) update_cloud(sky,cloud,elapsed);
}

/* Sky color for a normalized time of day.
 */
 
static uint32_t sky_color(double p) {

  // First, convert to 0..1=midnight..noon. We don't care which half we're in.
  if (p>=0.75) p-=0.75;
  else if (p>=0.25) p=0.75-p;
  else p+=0.25;
  p*=2.0;
  
  // Then apply this one-dimensional gradient:
  const struct gstop { double p,r,g,b; } gstopv[]={
    {0.000,0.251,0.169,0.384}, // midnight
    {0.470,0.302,0.294,0.545}, // night
    {0.500,0.753,0.506,0.753}, // daybreak
    {0.530,0.451,0.631,0.737}, // day
    {1.000,0.631,0.839,0.894}, // noon
  };
  const int gstopc=sizeof(gstopv)/sizeof(gstopv[0]);
  const struct gstop *lo=gstopv+gstopc-1;
  const struct gstop *hi=lo;
  int i=0; for (;i<gstopc;i++) {
    if (p<gstopv[i].p) {
      hi=gstopv+i;
      if (i) lo=hi-1; else lo=hi;
      break;
    }
  }
  double r,g,b;
  if (lo==hi) { // At noon or midnight, we might have only a single endpoint.
    r=lo->r;
    g=lo->g;
    b=lo->b;
  } else { // Normally, we have (gstopp) as the low bound and (gstopp+1) as the high bound.
    double hiweight=(p-lo->p)/(hi->p-lo->p);
    if (hiweight<0.0) hiweight=0.0;
    else if (hiweight>1.0) hiweight=1.0;
    double loweight=1.0-hiweight;
    r=lo->r*loweight+hi->r*hiweight;
    g=lo->g*loweight+hi->g*hiweight;
    b=lo->b*loweight+hi->b*hiweight;
  }

  // Finally, quantize and pack.
  int ri=(int)(r*255.0); if (ri<0) ri=0; else if (ri>0xff) ri=0xff;
  int gi=(int)(g*255.0); if (gi<0) gi=0; else if (gi>0xff) gi=0xff;
  int bi=(int)(b*255.0); if (bi<0) bi=0; else if (bi>0xff) bi=0xff;
  return (ri<<24)|(gi<<16)|(bi<<8)|0xff;
}

/* Sun or moon's position in screen pixels, from dayp in 0..1/2.
 * Returns the center of the body.
 */
 
static void sky_get_sun_position(int *x,int *y,double p) {
  // Have them follow a circular path (deasil), diameter a bit less than the screen width, and offset down so the vertical center is well off-screen.
  double t=p*M_PI*2.0;
  double r=(FBW-(NS_sys_tilesize<<1))*0.5;
  *x=(FBW>>1)+(int)(cos(t)*r);
  *y=FBH+(NS_sys_tilesize)-(int)(sin(t)*r);
}

/* Render.
 */
 
void sky_render(struct sky *sky) {

  /* Background.
   */
  uint32_t bgcolor=sky_color(sky->dayp);
  graf_draw_rect(&g.graf,0,0,FBW,FBH,bgcolor);
  
  /* Stars.
   */
  if (sky->dayp>=0.500) {
    const uint32_t stargb=0xffff0000;
    struct star *star=sky->starv;
    int i=STAR_COUNT;
    for (;i-->0;star++) {
      if (sky->dayp<=star->ta) continue;
      if (sky->dayp>=star->tz) continue;
      uint8_t alpha=0xff;
      if (sky->dayp<star->fa) { // fade in
        alpha=(int)((255.0*(sky->dayp-star->ta))/(star->fa-star->ta));
      } else if (sky->dayp>star->fz) { // fade out
        alpha=(int)((255.0*(star->tz-sky->dayp))/(star->tz-star->fz));
      }
      graf_draw_rect(&g.graf,star->x,star->y,1,1,stargb|alpha);
    }
  }
  
  /* Sun or moon.
   */
  if (sky->dayp>=0.500) {
    int dstx,dsty; sky_get_sun_position(&dstx,&dsty,sky->dayp-0.500);
    graf_draw_decal(&g.graf,g.texid_tiles,dstx-NS_sys_tilesize,dsty-NS_sys_tilesize,NS_sys_tilesize*4,NS_sys_tilesize*8,NS_sys_tilesize*2,NS_sys_tilesize*2,0);
  } else {
    int dstx,dsty; sky_get_sun_position(&dstx,&dsty,sky->dayp);
    graf_draw_mode7(&g.graf,g.texid_tiles,dstx,dsty,NS_sys_tilesize*2,NS_sys_tilesize*8,NS_sys_tilesize*2,NS_sys_tilesize*2,1.0,1.0,-sky->sunt,1);
    graf_draw_mode7(&g.graf,g.texid_tiles,dstx,dsty,0,NS_sys_tilesize*8,NS_sys_tilesize*2,NS_sys_tilesize*2,1.0,1.0,sky->sunt,1);
  }
  
  /* Clouds.
   */
  graf_set_alpha(&g.graf,0x80);
  graf_set_tint(&g.graf,(bgcolor&0xffffff00)|0xc0);
  struct cloud *cloud=sky->cloudv;
  int i=CLOUD_COUNT;
  for (;i-->0;cloud++) {
    struct puff *puff=cloud->puffv;
    int ii=PUFF_COUNT;
    for (;ii-->0;puff++) {
      int x=(int)(cloud->x+puff->x);
      int y=(int)(cloud->y+puff->y);
      graf_draw_tile(&g.graf,g.texid_tiles,x,y,puff->tileid,puff->xform);
    }
  }
  graf_set_alpha(&g.graf,0xff);
  graf_set_tint(&g.graf,0);
}
