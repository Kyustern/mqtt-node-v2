#include "watchdog_service.h"
#include "config.h"

WatchdogService watchdogService;

// Constructor
WatchdogService::WatchdogService() 
  : _bootTime(0), 
    _lastCheckTime(0) {
}

// Initialize watchdog
void WatchdogService::begin() {
  _bootTime = millis();
  _lastCheckTime = _bootTime;
  DEBUG_PRINTLN(F("Watchdog service initialized"));
}

// Check for millis overflow (call in main loop)
void WatchdogService::check() {
  unsigned long now = millis();
  
  // Check periodically, not every loop iteration
  if (now - _lastCheckTime < WATCHDOG_CHECK_INTERVAL_MS) {
    return;
  }
  _lastCheckTime = now;
  
  // Check if overflow threshold has been reached
  if (hasOverflowOccurred()) {
    DEBUG_PRINTLN(F("Millis overflow threshold reached! Rebooting..."));
    triggerReboot();
  }
}

// Trigger immediate reboot
void WatchdogService::triggerReboot() {
  DEBUG_PRINTLN(F("Triggering reboot..."));
  ESP.restart();
  // Should never reach here
  while (true) delay(1000);
}

// Get time since boot
unsigned long WatchdogService::getUptime() const {
  return millis() - _bootTime;
}

// Get remaining time until overflow
unsigned long WatchdogService::getTimeUntilOverflow() const {
  unsigned long elapsed = millis() - _bootTime;
  if (elapsed > MILLIS_OVERFLOW_THRESHOLD) {
    return 0;
  }
  return MILLIS_OVERFLOW_THRESHOLD - elapsed;
}

// Check if overflow threshold has been reached
bool WatchdogService::hasOverflowOccurred() const {
  unsigned long elapsed = millis() - _bootTime;
  // Account for millis() wrapping
  if (millis() < _bootTime) {
    // millis() has wrapped, so we've definitely passed the threshold
    return true;
  }
  return elapsed >= MILLIS_OVERFLOW_THRESHOLD;
}
