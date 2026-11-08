#pragma once

#include <ESP8266WebServer.h>
#include "types.h"
#include "config.h"

// Forward declarations
class BufferService;
class LEDService;

class APIService {
public:
  // Constructor
  APIService();
  
  // Initialize with services
  void begin(BufferService& buffer, LEDService& led);
  
  // Start API server
  void startServer();
  
  // Stop API server
  void stopServer();
  
  // Process API requests (call in main loop)
  void loop();
  
  // Check if server is running
  bool isRunning() const;
  
  // Set callbacks for actions that need to be handled by main
  void setRebootCallback(void (*callback)());
  void setRebootCleanCallback(void (*callback)());
  void setOTACallback(void (*callback)());
  void setDeviceInfoCallback(DeviceInfo (*callback)());
  
private:
  ESP8266WebServer _server;
  BufferService* _bufferService;
  LEDService* _ledService;
  bool _running;
  
  // Callbacks
  void (*_rebootCallback)();
  void (*_rebootCleanCallback)();
  void (*_otaCallback)();
  DeviceInfo (*_deviceInfoCallback)();
  
  // Setup routes
  void setupRoutes();
  
  // API Endpoint Handlers
  void handleTime();
  void handleCurrent();
  void handleData();
  void handleInfo();
  void handleLED();
  void handleReboot();
  void handleRebootClean();
  void handleOTA();
  
  // Helper functions
  void sendJsonResponse(int code, const String& json);
  void sendErrorResponse(int code, const String& message);
  String sensorReadingToJson(const SensorReading& reading) const;
  String deviceInfoToJson(const DeviceInfo& info) const;
};

// Global instance
extern APIService apiService;
