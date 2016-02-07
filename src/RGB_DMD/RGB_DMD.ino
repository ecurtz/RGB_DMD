// Copyright (c) 2015 Eli Curtz

#include <SmartMatrix3.h>
#include <MatrixHardware_RGB_DMD.h>

#include "BoardInfo.h"
#include "VersionInfo.h"
#include "Watchdog.h"
#include "Palettes.h"
#include "InputUSB.h"
#include "InputDMD.h"
#include "SplashScreen.h"
#include "PinballBrowser.h"
#include "MatrixSetup.h"

bool dmdInput = false;
bool usbInput = false;

// Palette data
// This is a default palette
// Defined here for convenience during testing
rgb24 testPalette[] = {
  {0x00, 0x00, 0x00}, // Used by 4 color DMD
  {0x1E, 0x1E, 0x1E}, // Nothing below 0x1E will survive color correction
  {0x26, 0x26, 0x26}, // so these two entries are cheated a little
  {0x33, 0x33, 0x33},
  {0x44, 0x44, 0x44},
  {0x55, 0x55, 0x55}, // Used by 4 color DMD
  {0x66, 0x66, 0x66},
  {0x77, 0x77, 0x77},
  {0x88, 0x88, 0x88},
  {0x99, 0x99, 0x99},
  {0xAA, 0xAA, 0xAA}, // Used by 4 color DMD
  {0xBB, 0xBB, 0xBB},
  {0xCC, 0xCC, 0xCC},
  {0xDD, 0xDD, 0xDD},
  {0xEE, 0xEE, 0xEE},
  {0xFF, 0xFF, 0xFF}  // Used by 4 color DMD
};

/*
 Initialize code sections, show splash, and wait for input from connected dmd or usb
*/
void setup()
{
  // Initialize hardware pins
  pinMode(MOSI, OUTPUT);
//  pinMode(GPIO_PIN_OE_TEENSY_PIN, OUTPUT);
//  digitalWriteFast(GPIO_PIN_OE_TEENSY_PIN, HIGH);
  // Initialize watchdog timer
  initializeWatchdog();
  // Initialize USB serial communication
  initializeInputUSB();
  // Initialize DMD communication
  initializeInputDMD();
  // Initialize software version information
  initializeVersionInfo();
  // Initialize hardware ID information
  initializeBoardInfo();
  // Initialize settings from EEPROM
  //initializeSettings();
  // Initialize Pinball Browser interface - must be AFTER initializeBoardInfo() for ID check
  initializePinballBrowser();
  
  // Configure matrix output
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();
  matrix.setBrightness(64);
  matrix.setRefreshRate(120);
  backgroundLayer.enableColorCorrection(false);

  // Set a temporary test palette
  for (int i = 0; i < 16; i++) {
     colorCorrection(testPalette[i], globalPalette[i]);
  }
  setActivePalette(0);
  
  // Display splash screen
  delay(20);
  showSplashScreen(true);
  delay(1500);
  kickWatchdog();
  delay(1500);

  // Dump a little information
  printResetType();
  Serial.print("Hardware ID: "); Serial.println(idString);
  Serial.print("Software version: "); Serial.println(versionString);

  // Wait for USB or DMD data
  do {
    kickWatchdog();
    dmdInput = hasInputDMD();
    usbInput = hasInputUSB();
  } while (!dmdInput && !usbInput);

  // Clear the Screen
  backgroundLayer.fillScreen(globalPalette[0]);
  backgroundLayer.swapBuffers();
  backgroundLayer.fillScreen(globalPalette[0]);
  
  // Configure active input
  if (dmdInput) {
   configureInputDMD(); 
  }
  if (usbInput) {
    configureInputUSB();
  }

  Serial.println("Setup complete");
}

/*
 Repeat forever. Look for input, kick watchdog
*/
void loop()
{
  if ((dmdInput && handleInputDMD()) || (usbInput && handleInputUSB())) {
    kickWatchdog();
  }
}
