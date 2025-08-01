#ifndef GAME_H
#define GAME_H

#include "shovel/shovel.h"
#include "opt/r1b/r1b.h"
#include <stdint.h>

#define FBW 64
#define FBH 36

extern struct g {
  int pvinput;
  uint32_t fb[FBW*FBH];
} g;

#endif
