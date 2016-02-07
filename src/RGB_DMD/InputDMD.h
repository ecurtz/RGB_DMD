// Copyright (c) 2015 Eli Curtz

#ifndef InputDMD_h
#define InputDMD_h

enum {
  FORMAT_UNKNOWN = 0,
  FORMAT_WPC,
  FORMAT_WHITESTAR,
  FORMAT_SAM,
  FORMAT_PROC_BOOT,
};

extern uint8_t dmdDataFormat;

void initializeInputDMD();

bool hasInputDMD();

void configureInputDMD();

bool handleInputDMD();

#endif
