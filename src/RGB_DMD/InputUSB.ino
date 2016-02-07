// Copyright (c) 2015 Eli Curtz

#include "InputUSB.h"
#include "Settings.h"

#define USB_BUFFER_SIZE 64

uint8_t usbBuffer[USB_BUFFER_SIZE];
USBCommandHeader usbCommand;

/*
 Open USB serial connection
*/
void initializeInputUSB()
{
  usbInputIndex = 0;
  
  Serial.begin(115200);
  delay(250);

  globalPalette[0] = rgb24(0x00, 0x00, 0x00);
  globalPalette[1] = rgb24(0x1E, 0x1E, 0x1E); // Nothing below 0x1E will survive color correction
  globalPalette[2] = rgb24(0x26, 0x26, 0x26); // so these two entries are cheated a little
  globalPalette[3] = rgb24(0x33, 0x33, 0x33);
  globalPalette[4] = rgb24(0x44, 0x44, 0x44);
  globalPalette[5] = rgb24(0x55, 0x55, 0x55);
  globalPalette[6] = rgb24(0x66, 0x66, 0x66);
  globalPalette[7] = rgb24(0x77, 0x77, 0x77);
  globalPalette[8] = rgb24(0x88, 0x88, 0x88);
  globalPalette[9] = rgb24(0x99, 0x99, 0x99);
  globalPalette[10] = rgb24(0xAA, 0xAA, 0xAA);
  globalPalette[11] = rgb24(0xBB, 0xBB, 0xBB);
  globalPalette[12] = rgb24(0xCC, 0xCC, 0xCC);
  globalPalette[13] = rgb24(0xDD, 0xDD, 0xDD);
  globalPalette[14] = rgb24(0xEE, 0xEE, 0xEE);
  globalPalette[15] = rgb24(0xFF, 0xFF, 0xFF);

  setActivePalette(0);
}

/*
 Check for USB signal from host
*/
bool hasInputUSB()
{
  return (Serial.dtr() && handleInputUSB());
}

/*
 Configure USB serial connection
*/
void configureInputUSB()
{
}

/*
 Read USB input into commands
*/
bool handleInputUSB()
{
  bool gotCommand = false;
  
  while (Serial.available()) {
    if (usbInputIndex < USB_MARKER_LENGTH) {
      usbCommand.marker[usbInputIndex] = Serial.read();
      if (usbCommand.marker[usbInputIndex] == usbMarker[usbInputIndex]) {
        usbInputIndex++;
      }
      else {
        usbInputIndex = 0;
      }
    }
    else if (usbInputIndex < sizeof(USBCommandHeader)) {
      uint8_t* commandPtr = (uint8_t*) &usbCommand;
      commandPtr[usbInputIndex] = Serial.read();
      usbInputIndex++;
    }
    else {
      int32_t dataReceived = usbInputIndex - sizeof(USBCommandHeader);
      int32_t dataAvailable = Serial.available();
      int32_t dataLength = 0;
      int32_t dataRemaining = 0;
      int32_t dataRead = 0;
      uint8_t* dataPtr = usbBuffer;
      rgb24* pixelPtr = backgroundLayer.backBuffer();

      switch (usbCommand.command) {
        case CMD_ACK:
          // argument: none
          // data: none
          Serial.println("ACK");
          break;
        case CMD_GET_VERS:
          // argument: none
          // data: none
          Serial.println(versionString);
          break;
        case CMD_GET_ID:
          // argument: none
          // data: none
          Serial.println(idString);
          break;
        case CMD_ROW_DATA_RGB:
          // argument: row number
          // data: COL_COUNT of RGB24 pixels
          if (usbCommand.argument < ROW_COUNT) {
            dataLength = COL_COUNT * sizeof(rgb24);
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataPtr = ((uint8_t*) (backgroundLayer.backBuffer() + usbCommand.argument * COL_COUNT)) + dataReceived;
            Serial.readBytes((char*) dataPtr, dataRead);
          }
          break;
        case CMD_SCREEN_DATA_RGB:
          // argument: none
          // data: ROW_COUNT * COL_COUNT of RGB24 pixels
          dataLength = ROW_COUNT * COL_COUNT * sizeof(rgb24);
          dataRemaining = dataLength - dataReceived;
          dataRead = min(dataAvailable, dataRemaining);
          dataPtr = ((uint8_t*) backgroundLayer.backBuffer()) + dataReceived;
          Serial.readBytes((char*) dataPtr, dataRead);
          if (dataRead == dataRemaining) {
            backgroundLayer.swapBuffers(false);         
          }
          break;
        case CMD_ROW_DATA_8:
          // argument: row number
          // data: COL_COUNT of 8bit palette indices
          if (usbCommand.argument < ROW_COUNT) {
            dataLength = COL_COUNT;
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataRead = min(dataRead, USB_BUFFER_SIZE);
            Serial.readBytes((char*) dataPtr, dataRead);
            pixelPtr += (usbCommand.argument * COL_COUNT) + dataReceived;
            for (int i = 0; i < dataRead; i++) {
              *pixelPtr++ = globalPalette[usbBuffer[i]];
            }
          }
          break;
        case CMD_SCREEN_DATA_8:
          // argument: none
          // data: ROW_COUNT * COL_COUNT of 8bit palette indices
          dataLength = ROW_COUNT * COL_COUNT;
          dataRemaining = dataLength - dataReceived;
          dataRead = min(dataAvailable, dataRemaining);
          dataRead = min(dataRead, USB_BUFFER_SIZE);
          Serial.readBytes((char*) dataPtr, dataRead);
          pixelPtr += dataReceived;
          for (int i = 0; i < dataRead; i++) {
            *pixelPtr++ = globalPalette[usbBuffer[i]];
          }
          if (dataRead == dataRemaining) {
            backgroundLayer.swapBuffers(false);         
          }
        break;
        case CMD_ROW_DATA_4:
          // argument: row number
          // data: COL_COUNT of 4bit palette indices
          if (usbCommand.argument < ROW_COUNT) {
            dataLength = COL_COUNT / 2;
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataRead = min(dataRead, USB_BUFFER_SIZE);
            Serial.readBytes((char*) dataPtr, dataRead);
            pixelPtr += (usbCommand.argument * COL_COUNT) + (dataReceived * 2);
            for (int i = 0; i < dataRead; i++) {
              *pixelPtr++ = activePalette[usbBuffer[i] >> 4];
              *pixelPtr++ = activePalette[usbBuffer[i] & 0x0F];
            }
          }
          break;
        case CMD_SCREEN_DATA_4:
          // argument: none
          // data: ROW_COUNT * COL_COUNT of 4bit palette indices
          dataLength = ROW_COUNT * COL_COUNT / 2;
          dataRemaining = dataLength - dataReceived;
          dataRead = min(dataAvailable, dataRemaining);
          dataRead = min(dataRead, USB_BUFFER_SIZE);
          Serial.readBytes((char*) dataPtr, dataRead);
          pixelPtr += dataReceived * 2;
          for (int i = 0; i < dataRead; i++) {
            *pixelPtr++ = activePalette[usbBuffer[i] >> 4];
            *pixelPtr++ = activePalette[usbBuffer[i] & 0x0F];
          }
          if (dataRead == dataRemaining) {
            backgroundLayer.swapBuffers(false);         
          }
         break;
        case CMD_PLANE_DATA:
          // argument: plane number
          // data: ROW_COUNT * COL_COUNT bits of the plane
          if (usbCommand.argument < PLANE_COUNT) {
            dataLength = ROW_COUNT * COL_COUNT / 8;
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataPtr = (uint8_t*) planes[usbCommand.argument];
            dataPtr += dataReceived;
            Serial.readBytes((char*) dataPtr, dataRead);
         }
         break;
        case CMD_FLIP_BUFFER:
          // argument: none
          // data: none
          backgroundLayer.swapBuffers(false);
          break;
        case CMD_SET_PALETTE:
          // argument: palette number
          // data: PALETTE_ENTRIES of RGB24 colors
          if (usbCommand.argument < GLOBAL_PALETTE_COUNT) {
            dataLength = PALETTE_ENTRIES * sizeof(rgb24);
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataPtr = ((uint8_t*) &globalPalette[usbCommand.argument * PALETTE_ENTRIES]) + dataReceived;
            Serial.readBytes((char*) dataPtr, dataRead);
          }
          break;
        case CMD_SAVE_PALETTE:
          // argument: palette number
          // data: PALETTE_NAME_LENGTH of char 
          if (usbCommand.argument < GLOBAL_PALETTE_COUNT) {
            dataLength = PALETTE_NAME_LENGTH;
            dataRemaining = dataLength - dataReceived;
            dataRead = min(dataAvailable, dataRemaining);
            dataPtr += dataReceived;
            Serial.readBytes((char*) dataPtr, dataRead);
            if (dataRead == dataRemaining) {
              PalleteInfo tempPalette;
              memcpy(tempPalette.name, usbBuffer, PALETTE_NAME_LENGTH);
              memcpy(tempPalette.colors, &globalPalette[usbCommand.argument * PALETTE_ENTRIES], sizeof(rgb24) * PALETTE_ENTRIES);
              savePalette(usbCommand.argument, tempPalette);
            }
          }
          break;
        case CMD_ACTIVATE_PALETTE:
          // argument: palette number
          // data: none
          setActivePalette(usbCommand.argument);
          break;
        case CMD_REFRESH_RATE:
          // argument: desired framerate
          // data: none
//           setRefreshRate(usbCommand.argument);
         break;
        case CMD_SAVE_SETTINGS:
          // argument: none
          // data: none
          saveSettings();
          break;
        case CMD_RESTART:
          // argument: none
          // data: none
          Serial.println("RESETTING BOARD...");
          resetBoard();
          break;
        default:
          Serial.print("ERROR: BAD COMMAND: ");
          Serial.println(usbCommand.command);
          break;
      }

      usbInputIndex += dataRead;
      dataRemaining -= dataRead;
      if (dataRemaining <= 0) {
        gotCommand = true;
        usbInputIndex = 0;
      }
    }
  }

  return gotCommand;
}


