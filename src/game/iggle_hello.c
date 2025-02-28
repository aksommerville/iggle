#include "iggle.h"

struct hello {
  int TODO;
} hello={0};

/* End.
 */
 
void hello_end() {
}

/* Begin.
 */
 
int hello_begin() {
  memset(&hello,0,sizeof(struct hello));
  return 0;
}

/* Update.
 */
 
void hello_update(double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    iggle_set_mode(IGGLE_MODE_PLAY);
  }
}

/* Render.
 */
 
void hello_render() {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0xffc050ff);
}
