#include "BoardInfo.h"

uint8_t boardID[6];
char idString[7];

/*
 Read the unique MAC address of the Teensy into boardID
 Build a string out of the 3 NIC bytes
*/
void initializeBoardInfo()
{
  readMAC(0xe, boardID, 0);
  readMAC(0xf, boardID, 3);

  memset(idString, 0, sizeof(idString));
  if (boardID[3] < 0x10) {
    idString[0] = '0';
    itoa(boardID[3], &idString[1], 16);
  }
  else {
    itoa(boardID[3], &idString[0], 16);
  }
  
  if (boardID[4] < 0x10) {
    idString[2] = '0';
    itoa(boardID[4], &idString[3], 16);
  }
  else {
    itoa(boardID[4], &idString[2], 16);
  }
  
  if (boardID[5] < 0x10) {
    idString[4] = '0';
    itoa(boardID[5], &idString[5], 16);
  }
  else {
    itoa(boardID[5], &idString[4], 16);
  }
}

/*
 Read the MAC address
 http://forum.pjrc.com/threads/91-teensy-3-MAC-address
 To understand what's going on here, see
 "Kinetis Peripheral Module Quick Reference" page 85 and
 "K20 Sub-Family Reference Manual" page 548.
*/
void readMAC(uint8_t word, uint8_t *mac, uint8_t offset) {
  noInterrupts();         // Disable interrupts
  FTFL_FCCOB0 = 0x41;     // Selects the READONCE command
  FTFL_FCCOB1 = word;     // Read the given word of read once area

  // launch command and wait until complete
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while(!(FTFL_FSTAT & FTFL_FSTAT_CCIF));

  *(mac+offset) =   FTFL_FCCOB5;    // Collect only the top three bytes,
  *(mac+offset+1) = FTFL_FCCOB6;    // in the right orientation (big endian).
  *(mac+offset+2) = FTFL_FCCOB7;    // Skip FTFL_FCCOB4 as it's always 0.
  interrupts();                     // Reenable interrupts
}

/*
 Reboot the board
 http://forum.pjrc.com/threads/24304-_reboot_Teensyduino()-vs-_restart_Teensyduino()?p=45253
*/
void resetBoard()
{
  while(Serial.available()) {
    Serial.read();
  }
  Serial.flush();
  delay(100);

  SCB_AIRCR = 0x5FA0004;
}

