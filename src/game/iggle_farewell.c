#include "iggle.h"

struct farewell {
  int TODO;
} farewell={0};

/* End.
 */
 
void farewell_end() {
}

/* Begin.
 */
 
int farewell_begin() {
  return 0;
}

/* Update.
 */
 
void farewell_update(double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    iggle_set_mode(IGGLE_MODE_HELLO);
  }
}

/* Render.
 */
 
void farewell_render() {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x800000ff);
}
