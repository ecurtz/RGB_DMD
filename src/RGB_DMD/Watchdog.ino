#include "Watchdog.h"

#define MIN_WATCHDOG_DELAY 32

unsigned long nextWatchdogKick;

#ifdef __cplusplus
extern "C" {
#endif
/*
 Enable the watchdog timer
 https://forum.pjrc.com/threads/25370-Teensy-3-0-Watchdog-Timer
*/
void startup_early_hook()
{
  // The next 2 lines set the timeout value. This is the value that the watchdog timer compare itself to.
  WDOG_TOVALL = 2000;
  WDOG_TOVALH = 0;
  // Enable watchdog
  WDOG_STCTRLH = (WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN);
}

#ifdef __cplusplus
}
#endif

/*
 Set the initial watchdog kick time
*/
void initializeWatchdog()
{
  nextWatchdogKick = millis() + MIN_WATCHDOG_DELAY;
}

/*
 Kick the watchdog timer
*/
void kickWatchdog()
{
  // Wraparound safe test for elapsed time
  if((long)( millis() - nextWatchdogKick ) >= 0)
  {
    nextWatchdogKick = millis() + MIN_WATCHDOG_DELAY;
    noInterrupts();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    interrupts();
  }
}

/*
 Display debug information about the last cpu reset
*/
void printResetType()
{
    if (RCM_SRS1 & RCM_SRS1_SACKERR)   Serial.println("[RCM_SRS1] - Stop Mode Acknowledge Error Reset");
    if (RCM_SRS1 & RCM_SRS1_MDM_AP)    Serial.println("[RCM_SRS1] - MDM-AP Reset");
    if (RCM_SRS1 & RCM_SRS1_SW)        Serial.println("[RCM_SRS1] - Software Reset");
    if (RCM_SRS1 & RCM_SRS1_LOCKUP)    Serial.println("[RCM_SRS1] - Core Lockup Event Reset");
    if (RCM_SRS0 & RCM_SRS0_POR)       Serial.println("[RCM_SRS0] - Power-on Reset");
    if (RCM_SRS0 & RCM_SRS0_PIN)       Serial.println("[RCM_SRS0] - External Pin Reset");
    if (RCM_SRS0 & RCM_SRS0_WDOG)      Serial.println("[RCM_SRS0] - Watchdog(COP) Reset");
    if (RCM_SRS0 & RCM_SRS0_LOC)       Serial.println("[RCM_SRS0] - Loss of External Clock Reset");
    if (RCM_SRS0 & RCM_SRS0_LOL)       Serial.println("[RCM_SRS0] - Loss of Lock in PLL Reset");
    if (RCM_SRS0 & RCM_SRS0_LVD)       Serial.println("[RCM_SRS0] - Low-voltage Detect Reset");
}
