#pragma once

#include <ArduinoOTA.h>
#include "types.h"
#include "config.h"

class OTAService {
public:
  // Constructor
  OTAService();
  
  // Initialize OTA service
  void begin(const String& hostname = OTA_HOSTNAME);
  
  // Start OTA server
  void start();
  
  // Stop OTA server
  void stop();
  
  // Process OTA events (call in main loop)
  void loop();
  
  // Check if OTA is in progress
  bool isUpdating() const;
  
  // Get OTA progress percentage
  uint8_t getProgress() const;
  
  // Get OTA status message
  String getStatus() const;
  
private:
  bool _started;
  bool _updating;
  uint8_t _progress;
  String _status;
  
  // OTA event handlers
  void setupOTA();
  
  static void _onStart();
  static void _onEnd();
  static void _onProgress(unsigned int progress, unsigned int total);
  static void _onError(ota_error_t error);
};

// Global instance
extern OTAService otaService;
