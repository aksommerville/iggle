#ifndef IGGLE_H
#define IGGLE_H

#define FBW 640
#define FBH 360

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/text/text.h"
#include "egg_rom_toc.h"
#include "shared_symbols.h"

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct texcache texcache;
  struct font *font;
} g;

#endif
