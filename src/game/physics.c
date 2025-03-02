#include "iggle.h"

/* Apply correction.
 */
 
static void physics_correct_for_cell(double *l,double *t,double *r,double *b,struct sprite *sprite,double corrx,double corry,int col,int row) {
  if (corrx<0.0) {
    *r=col;
    *l=(*r)-sprite->pw;
    sprite->x=(*l)-sprite->px;
  } else if (corrx>0.0) {
    *l=col+1.0;
    *r=(*l)+sprite->pw;
    sprite->x=(*l)-sprite->px;
  } else if (corry<0.0) {
    *b=row;
    *t=(*b)-sprite->ph;
    sprite->y=(*t)-sprite->py;
  } else if (corry>0.0) {
    *t=row+1.0;
    *b=(*t)+sprite->ph;
    sprite->y=(*t)-sprite->py;
  }
}
 
static void physics_correct_for_box(double *l,double *t,double *r,double *b,struct sprite *sprite,double corrx,double corry,double ol,double ot,double or,double ob) {
  if (corrx<0.0) {
    *r=ol;
    *l=(*r)-sprite->pw;
    sprite->x=(*l)-sprite->px;
  } else if (corrx>0.0) {
    *l=or;
    *r=(*l)+sprite->pw;
    sprite->x=(*l)-sprite->px;
  } else if (corry<0.0) {
    *b=ot;
    *t=(*b)-sprite->ph;
    sprite->y=(*t)-sprite->py;
  } else if (corry>0.0) {
    *t=ob;
    *b=(*t)+sprite->ph;
    sprite->y=(*t)-sprite->py;
  }
}

/* Resolve collisions for one sprite.
 */
 
int sprite_collide(struct sprite *sprite,double corrx,double corry) {
  if (!sprite) return 0;
  
  const double JUST_A_HAIR=0.000001;
  double l=sprite->x+sprite->px;
  double t=sprite->y+sprite->py;
  double r=l+sprite->pw-JUST_A_HAIR;
  double b=t+sprite->ph-JUST_A_HAIR;
  int result=0;
  
  /* Screen edges.
   */
  if (l<0.0) {
    sprite->x=-sprite->px;
    l=0.0;
    r=sprite->pw;
    result=1;
  } else if (r>NS_sys_mapw) {
    sprite->x=NS_sys_mapw-sprite->pw-sprite->px;
    r=NS_sys_mapw;
    l=r-sprite->pw;
    result=1;
  }
  if (t<0.0) {
    sprite->y=-sprite->py;
    t=0.0;
    b=sprite->ph;
    result=1;
  } else if (b>NS_sys_maph) {
    sprite->y=NS_sys_maph-sprite->ph-sprite->py;
    b=NS_sys_maph;
    t=b-sprite->ph;
    result=1;
  }
  
  /* Map.
   */
  int cola=(int)l; if (cola<0) cola=0;
  int colz=(int)r; if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowa=(int)t; if (rowa<0) rowa=0;
  int rowz=(int)b; if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    const uint8_t *cellsrow=play_get_map()+rowa*NS_sys_mapw+cola;
    int row=rowa; for (;row<=rowz;row++,cellsrow+=NS_sys_mapw) {
      const uint8_t *cell=cellsrow;
      int col=cola; for (;col<=colz;col++,cell++) {
        switch (g.physics[*cell]) {
          case NS_physics_solid:
          case NS_physics_goal: {
              physics_correct_for_cell(&l,&t,&r,&b,sprite,corrx,corry,col,row);
              result=1;
            } break;
        }
      }
    }
  }
  
  /* Solid sprites.
   */
  if (sprite->solid) {
    int i=spritec;
    while (i-->0) {
      const struct sprite *other=spritev[i];
      if (other->defunct) continue;
      if (other==sprite) continue;
      if (!other->solid) continue;
      double ol=other->x+other->px;
      if (r<=ol) continue;
      double or=ol+other->pw;
      if (l>=or) continue;
      double ot=other->y+other->py;
      if (b<=ot) continue;
      double ob=ot+other->ph;
      if (t>=ob) continue;
      physics_correct_for_box(&l,&t,&r,&b,sprite,corrx,corry,ol,ot,or,ob);
      result=1;
      break;
    }
  }
  
  return result;
}
