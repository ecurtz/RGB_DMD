// Copyright (c) 2015 Eli Curtz

#include "PinballBrowser.h"

bool matchingId = false;

void initializePinballBrowser()
{
  matchingId = true;
  for (int i = 0; i < 6; i++) {
    if (boardID[i] != pbData.id[i]) {
      matchingId = false;
    }
  }

  if (matchingId) {
    pbPaletteLoad = (void (*)(PBPaletteRequest *))((uint32_t) pbData.data1 | 1);
  }
}

bool hasPinballBrowser()
{
  return (matchingId && settings.pbActive);
}

#ifdef __cplusplus
extern "C" {
#endif

void nullPBFunction(PBPaletteRequest* request)
{
  request->index = -1;
}

#ifdef __cplusplus
}
#endif

