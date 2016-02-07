// Copyright (c) 2015 Eli Curtz

#include "VersionInfo.h"

char versionString[12];
uint8_t versionLength;

/*
 Build a string out of the version bytes
*/
void initializeVersionInfo()
{
  memset(versionString, 0, sizeof(versionString));
  
  versionLength = 0;
  itoa(versionMajor, versionString, 10);
  versionLength = strlen(versionString);
  versionString[versionLength++] = '.';
  itoa(versionMinor, &versionString[versionLength], 10);
  versionLength = strlen(versionString);
  versionString[versionLength++] = '.';
  itoa(versionBuild, &versionString[versionLength], 10);
  versionLength = strlen(versionString);
}

