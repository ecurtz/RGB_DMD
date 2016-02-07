// Copyright (c) 2015 Eli Curtz

#include "Palettes.h"

rgb24 globalPalette[256];
rgb24 *activePalette;

/*
 Set palette used for 4 and 16 color games
*/
void setActivePalette(uint8_t index)
{
  if (index < 16) {
    activePalette = &globalPalette[index * 16];
  }
  else {
    
  }
}

