#pragma once

#include "types.h"
#include "config.h"

class WatchdogService {
public:
  // Constructor
  WatchdogService();
  
  // Initialize watchdog
  void begin();
  
  // Check for millis overflow (call in main loop)
  void check();
  
  // Trigger immediate reboot
  void triggerReboot();
  
  // Get time since boot
  unsigned long getUptime() const;
  
  // Get remaining time until overflow
  unsigned long getTimeUntilOverflow() const;
  
private:
  unsigned long _bootTime;
  unsigned long _lastCheckTime;
  
  // Check if overflow threshold has been reached
  bool hasOverflowOccurred() const;
};

// Global instance
extern WatchdogService watchdogService;
