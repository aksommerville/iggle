/* XXX Just proving out the idea.
 * Don't keep any of this.
 */

#include "iggle.h"

struct body {
  double x,y; // m
  int vx,vy,vw,vh; // Render box in PIXELS relative to (x,y) after quantization.
  double px,py,pw,ph; // Hit box in METERS relative to (x,y) before quantization.
  uint32_t color;
};

static struct ta {
  struct body hero;
  struct body ground;
  struct body pumpkin;
  int button;
  int flap; // Model state of button. Mostly tracks (button) but also autoreleases.
  double button_clock; // Latches, holds value after release.
  double unbutton_clock;
  double velocity; // m/s
  double flydir; // 1.0,-1.0
  double gravity; // m/s
  double flap_blackout; // counts down after a bonk
  double pgravity;
  int grounded;
  int grabbed;
} ta={0};

void throwaway_init() {

  ta.ground=(struct body){
    .x=0.0,
    .y=10.0,
    .vw=FBW,
    .vh=NS_sys_tilesize*2,
    .pw=20.0,
    .ph=2.0,
    .color=0x004020ff,
  };
  ta.hero=(struct body){
    .x=10.0,
    .y=5.0,
    .vx=-(NS_sys_tilesize>>1),
    .vy=-(NS_sys_tilesize>>1),
    .vw=NS_sys_tilesize,
    .vh=NS_sys_tilesize,
    .px=-0.5,
    .py=-0.5,
    .pw=1.0,
    .ph=1.0,
    .color=0xffffffff,
  };
  ta.pumpkin=(struct body){
    .x=8.0,
    .y=9.5,
    .px=-0.5,
    .py=-0.5,
    .pw=1.0,
    .ph=1.0,
    .vx=-(NS_sys_tilesize>>1),
    .vy=-(NS_sys_tilesize>>1),
    .vw=NS_sys_tilesize,
    .vh=NS_sys_tilesize,
    .color=0xc06000ff,
  };

  ta.velocity=2.0;
  ta.flydir=1.0;
  ta.unbutton_clock=99.999; // above any threshold so we don't think the button was just released
}

static void check_turnaround() {
  int collision=0;
  if (ta.flydir>0.0) {
    double x=ta.hero.x+ta.hero.px+ta.hero.pw;
    if (x>=NS_sys_mapw) collision=1;
  } else {
    double x=ta.hero.x+ta.hero.px;
    if (x<=0.0) collision=1;
  }
  if (!collision) return;
  ta.flydir*=-1.0;
}

static void check_head() {
  // Way cheating this; in real life it will be much more involved.
  double herot=ta.hero.y+ta.hero.py;
  if (herot<=0.0) {
    ta.hero.y=-ta.hero.py;
    ta.velocity*=0.500;
    ta.gravity=6.0;
    ta.pgravity=8.0;
    ta.flap=0;
    ta.flap_blackout=0.250;
    ta.button_clock=ta.unbutton_clock=99.999;
    ta.grabbed=0;
  }
}

static void check_floor() {
  // Way cheating this; in real life it will be much more involved.
  double herob=ta.hero.y+ta.hero.py+ta.hero.ph;
  if (ta.grabbed) {
    herob+=ta.pumpkin.ph;
  }
  double groundt=ta.ground.y+ta.ground.py;
  if (herob>=groundt) {
    ta.hero.y=groundt-ta.hero.ph-ta.hero.py;
    if (ta.grabbed) {
      ta.hero.y-=ta.pumpkin.ph;
    }
    ta.grounded=1;
    return;
  }
  if (!ta.grabbed&&(ta.flap_blackout<=0.0)) {
    double pnkt=ta.pumpkin.y+ta.pumpkin.py;
    if (herob>=pnkt) {
      double herol=ta.hero.x+ta.hero.px;
      double heror=herol+ta.hero.pw;
      double pnkl=ta.pumpkin.x+ta.pumpkin.px;
      double pnkr=pnkl+ta.pumpkin.pw;
      if ((herol<pnkr)&&(heror>pnkl)) {
        ta.hero.y=pnkt-ta.hero.ph-ta.hero.py;
        ta.hero.x=ta.pumpkin.x; // Debatable.
        ta.grounded=1;
        ta.grabbed=1;
        return;
      }
    }
  }
  ta.grounded=0;
}

static void drop_pumpkin(double elapsed) {
  const double rate=10.0; // m/s**2
  double limit=8.0; // m/s
  if ((ta.pgravity+=rate*elapsed)>limit) ta.pgravity=limit;
  ta.pumpkin.y+=ta.pgravity*elapsed;
  double pnkb=ta.pumpkin.y+ta.pumpkin.py+ta.pumpkin.ph;
  double groundt=ta.ground.y+ta.ground.py;
  if (pnkb>=groundt) {
    ta.pumpkin.y=groundt-ta.pumpkin.ph-ta.pumpkin.py;
    ta.pgravity=0.0;
    return;
  }
}

void throwaway_update(double elapsed,int input) {
  
  /* Track button.
   */
  if (ta.flap_blackout>0.0) {
    ta.flap_blackout-=elapsed;
    input=0;
  }
  if (input&EGG_BTN_SOUTH) {
    if (!ta.button&&!ta.flap) {
      ta.button=1;
      ta.flap=1;
      ta.button_clock=0.0;
    } else if (ta.flap) {
      ta.button_clock+=elapsed;
    }
  } else {
    if (ta.button) {
      if (ta.flap) {
        ta.flap=0;
        ta.button_clock+=elapsed;
        ta.unbutton_clock=0.0;
      }
      ta.button=0;
      // Fast taps are below 0.150 or so, mostly around 0.060.
    }
  }
  if (ta.flap&&(ta.button_clock>=0.400)) {
    ta.flap=0;
    ta.unbutton_clock=0.0;
  }
  if (!ta.flap) {
    ta.unbutton_clock+=elapsed;
  }
  
  /* Adjust horizontal velocity.
   * (velocity) increases up to some limit while button held.
   * Decreases back down to some limit while released.
   */
  if (!ta.grounded) {
    if (ta.flap) {
      const double rate=10.0; // m/s**2
      const double limit=12.0; // m/s
      if ((ta.velocity+=rate*elapsed)>=limit) ta.velocity=limit;
    } else {
      const double rate=-10.0; // m/s**2
      const double limit=2.0; // m/s, should be the initial speed too
      if ((ta.velocity+=rate*elapsed)<=limit) ta.velocity=limit;
    }
  
  /* Apply horizontal velocity.
   */
    ta.hero.x+=ta.velocity*ta.flydir*elapsed;
    check_turnaround();
  }
  
  /* Adjust vertical velocity.
   */
  if (ta.flap) {
    ta.grounded=0;
    ta.gravity=0.0;
    double power=0.200-ta.button_clock;
    if (power>0.0) {
      power*=20.0; // from (0..1/5) to m/s
      ta.hero.y-=power*elapsed;
    } else {
      // Flapping but out of juice. Hold steady vertical?
    }
    check_head();
  } else if ((ta.button_clock<0.200)&&(ta.unbutton_clock<0.200)) {
    // Continue upward motion after releasing button.
    ta.grounded=0;
    ta.gravity=0.0;
    double power=0.200-(ta.button_clock+ta.unbutton_clock);
    if (power>0.0) {
      power*=20.0; // from (0..1/5) to m/s
      ta.hero.y-=power*elapsed;
    } else {
      // Flapping but out of juice. Hold steady vertical?
    }
    check_head();
  } else {
    // Gravity.
    const double rate=10.0; // m/s**2
    double limit=8.0; // m/s
    if ((ta.gravity+=rate*elapsed)>limit) ta.gravity=limit;
    ta.hero.y+=ta.gravity*elapsed;
    check_floor();
  }
  
  if (ta.grabbed) {
    ta.hero.color=0xffc0a0ff;
    ta.pumpkin.y=ta.hero.y+ta.hero.py+ta.hero.ph-ta.pumpkin.py;
    ta.pumpkin.x=ta.hero.x;
    ta.pgravity=0.0;
  } else {
    ta.hero.color=0xffffffff;
    drop_pumpkin(elapsed);
  }
}

static void draw_body(struct body *body) {
  int x=(int)(body->x*NS_sys_tilesize),y=(int)(body->y*NS_sys_tilesize);
  graf_draw_rect(&g.graf,x+body->vx,y+body->vy,body->vw,body->vh,body->color);
}

void throwaway_render() {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x80a0e0ff);
  draw_body(&ta.ground);
  draw_body(&ta.hero);
  draw_body(&ta.pumpkin);
}
