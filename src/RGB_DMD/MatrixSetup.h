// Copyright (c) 2015 Eli Curtz

#ifndef MatrixSetup_h
#define MatrixSetup_h

// DMD display size
#define ROW_COUNT 32
#define COL_COUNT 128
#define ROW_LENGTH  (COL_COUNT / 16)
#define COLOR_DEPTH 24

// Matrix configuration
const uint8_t kMatrixWidth = COL_COUNT;
const uint8_t kMatrixHeight = ROW_COUNT;
const uint8_t kRefreshDepth = 24;
const uint8_t kDmaBufferRows = 4;
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#endif
