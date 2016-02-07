// Copyright (c) 2015 Eli Curtz

#ifndef InputUSB_h
#define InputUSB_h

enum {
  CMD_ACK = 0,
  CMD_GET_VERS,
  CMD_GET_ID,
  CMD_ROW_DATA_RGB,
  CMD_SCREEN_DATA_RGB,
  CMD_ROW_DATA_8,
  CMD_SCREEN_DATA_8,
  CMD_ROW_DATA_4,
  CMD_SCREEN_DATA_4,
  CMD_PLANE_DATA,
  CMD_FLIP_BUFFER,
  CMD_SET_PALETTE,
  CMD_SAVE_PALETTE,
  CMD_ACTIVATE_PALETTE,
  CMD_REFRESH_RATE,
  CMD_SAVE_SETTINGS,
  CMD_RESTART,

  CMD_COUNT
};

#define USB_MARKER_LENGTH 4

typedef struct {
  uint8_t marker[USB_MARKER_LENGTH];
  uint8_t command;
  uint8_t argument;
  uint16_t checksum;
} USBCommandHeader;

const uint8_t usbMarker[USB_MARKER_LENGTH] = {0xBA, 0x11, 0x00, 0x03};

uint32_t usbInputIndex;

void initializeInputUSB();

bool hasInputUSB();

void configureInputUSB();

bool handleInputUSB();

#endif
