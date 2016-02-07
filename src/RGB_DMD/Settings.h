// Copyright (c) 2015 Eli Curtz

#ifndef Settings_h
#define Settings_h

#include "Palettes.h"

typedef struct {
  uint8_t versionMajor;
  uint8_t versionMinor;
  uint16_t savedPalettes;
  uint8_t activePalette;
  uint8_t refreshRate;
  uint8_t brightness;
  uint8_t pbActive;
  uint8_t unused[24];
} Settings;

extern Settings settings;

void saveSettings();

void savePalette(uint8_t, PalleteInfo&);

void initializeSettings();

#endif
