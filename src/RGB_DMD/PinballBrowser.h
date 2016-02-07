// Copyright (c) 2015 Eli Curtz

#ifndef PinballBrowser_h
#define PinballBrowser_h

void initializePinballBrowser();

bool hasPinballBrowser();

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
 uint8_t id[6];
 int16_t index;
 void* data;
 rgb24 palette[16];
} PBPaletteRequest;

typedef struct {
  const char marker[16];
  uint8_t id[6];
  uint8_t data0[10];
  uint16_t data1[256];
  uint8_t data2[12288];
} PBDataBlock;

const PBDataBlock pbData = {"PB Data Block\0\0", {1, 2, 3, 4, 5, 6}, {}, {}, {}};

void nullPBFunction(PBPaletteRequest*);

void (*pbPaletteLoad)(PBPaletteRequest*) = nullPBFunction;

#ifdef __cplusplus
}
#endif

#endif
