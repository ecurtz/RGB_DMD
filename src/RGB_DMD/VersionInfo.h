// Copyright (c) 2015 Eli Curtz

#ifndef VersionInfo_h
#define VersionInfo_h

#include <Arduino.h>

const uint8_t versionMajor = 0;
const uint8_t versionMinor = 6;
const uint8_t versionBuild = 5;

extern char versionString[12];
extern uint8_t versionLength;

void initializeVersionInfo();

#endif
