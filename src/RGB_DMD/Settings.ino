// Copyright (c) 2015 Eli Curtz
#include <EEPROM.h>

#include "Settings.h"
#include "VersionInfo.h"

Settings settings;

/*
 Write any modified bytes of settings to EEPROM
*/
void saveSettings()
{
  // EEPROM is accessed as 8 single bytes
  uint8_t* settingsPtr = (uint8_t*) &settings;
  for (uint16_t i = 0; i < sizeof(Settings); i++) {
    if (settingsPtr[i] != EEPROM.read(i)) {
//      EEPROM.write(i, settingsPtr[i]);
    }
  }  
}

/*
 Write any modified bytes of palette to EEPROM
*/
void savePalette(uint8_t index, PalleteInfo& palette)
{
  // Only store 16 palettes
  if (index >= 16) return;
  
  // EEPROM is accessed as 8 single bytes
  uint8_t* palettePtr = (uint8_t*) &palette;
  uint16_t offset = sizeof(Settings) + (index * sizeof(PalleteInfo));
  for (uint16_t i = 0; i < sizeof(PalleteInfo); i++) {
    if (palettePtr[i] != EEPROM.read(offset + i)) {
//      EEPROM.write(offset + i, palettePtr[i]);
    }
  }  

  settings.savedPalettes |= 0x0001 << index;
  saveSettings();
}

/*
 Load settings and custom palettes from EEPROM
*/
void initializeSettings()
{
  // EEPROM is accessed as 8 single bytes
  uint8_t* settingsPtr = (uint8_t*) &settings;
  for (uint16_t i = 0; i < sizeof(Settings); i++) {
    settingsPtr[i] = EEPROM.read(i);
  }

  // Check for first write and set default values
  if ((settings.versionMajor == settings.unused[0]) && (settings.versionMinor == settings.unused[1])) {
    settings.versionMajor = versionMajor;
    settings.versionMinor = versionMinor;
    settings.savedPalettes = 0;
    settings.activePalette = 0;
    settings.refreshRate = 120;
    settings.brightness = 64;
    settings.pbActive = false;

    saveSettings();
  }

  // Check for version change
  if ((settings.versionMajor != versionMajor) || (settings.versionMinor != versionMinor)) {
    settings.versionMajor = versionMajor;
    settings.versionMinor = versionMinor;    

    saveSettings();
  }

  // Load any saved palettes
  for (uint16_t index = 0; index < 16; index++) {
    if (settings.savedPalettes & (0x0001 << index)) {
      PalleteInfo palette;
      uint8_t* palettePtr = (uint8_t*) &palette;
      uint16_t offset = sizeof(Settings) + (index * sizeof(PalleteInfo));
      for (uint16_t i = 0; i < sizeof(PalleteInfo); i++) {
        palettePtr[i] = EEPROM.read(offset + i);
      }

      memcpy(&globalPalette[index * 16], &palette.colors, 16 * sizeof(rgb24));
    }
  }
  setActivePalette(settings.activePalette);
}

