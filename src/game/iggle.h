#ifndef IGGLE_H
#define IGGLE_H

#define FBW 640
#define FBH 352

#define IGGLE_MODE_PLAY 1
#define IGGLE_MODE_HELLO 2
#define IGGLE_MODE_FAREWELL 3

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/text/text.h"
#include "opt/rom/rom.h"
#include "egg_rom_toc.h"
#include "shared_symbols.h"
#include "sprite.h"

extern struct g {

  void *rom;
  int romc;
  struct rom_res *resv; // Only contains resources we might use, eg no image or sound.
  int resc,resa;
  
  struct graf graf;
  struct font *font;
  int texid_tiles;
  int texid_map;
  int mode; // Do not modify directly; use iggle_set_mode().
  int pvinput;
} g;

int iggle_set_mode(int mode);

int iggle_res_get(void *dstpp,int tid,int rid);

void play_end();
int play_begin();
void play_update(double elapsed,int input,int pvinput);
void play_render();
int play_load_map(int mapid);

void hello_end();
int hello_begin();
void hello_update(double elapsed,int input,int pvinput);
void hello_render();

void farewell_end();
int farewell_begin();
void farewell_update(double elapsed,int input,int pvinput);
void farewell_render();

#endif
