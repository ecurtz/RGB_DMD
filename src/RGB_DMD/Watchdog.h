#ifndef Watchdog_h
#define Watchdog_h

#ifdef __cplusplus
extern "C" {
#endif
void startup_early_hook();
#ifdef __cplusplus
}
#endif

void initializeWatchdog();

void kickWatchdog();

void printResetType();

#endif
