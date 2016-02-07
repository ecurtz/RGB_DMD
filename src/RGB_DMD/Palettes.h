// Copyright (c) 2015 Eli Curtz

#ifndef Palettes_h
#define Palettes_h

#define PALETTE_NAME_LENGTH 15
#define PALETTE_ENTRIES 16
#define GLOBAL_PALETTE_COUNT 16

typedef struct {
  char name[PALETTE_NAME_LENGTH + 1];
  rgb24 colors[16];
} PalleteInfo;

extern rgb24 globalPalette[GLOBAL_PALETTE_COUNT * PALETTE_ENTRIES];
extern rgb24 *activePalette;

void setActivePalette(uint8_t index);

#endif
