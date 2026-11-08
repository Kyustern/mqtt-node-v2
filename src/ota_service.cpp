#include "ota_service.h"
#include "config.h"

OTAService otaService;

// Constructor
OTAService::OTAService() 
  : _started(false), 
    _updating(false), 
    _progress(0), 
    _status("") {
}

// Initialize OTA service
void OTAService::begin(const String& hostname) {
  setupOTA();
  ArduinoOTA.setHostname(hostname.c_str());
  _started = true;
  _status = F("Ready");
  DEBUG_PRINTLN(F("OTA service initialized"));
}

// Start OTA server
void OTAService::start() {
  if (!_started) {
    begin();
  }
  ArduinoOTA.begin();
  _status = F("Server started on port ") + String(OTA_PORT);
  DEBUG_PRINTLN(_status);
}

// Stop OTA server
void OTAService::stop() {
  ArduinoOTA.end();
  _updating = false;
  _progress = 0;
  _status = F("Server stopped");
  DEBUG_PRINTLN(F("OTA server stopped"));
}

// Process OTA events (call in main loop)
void OTAService::loop() {
  if (_started) {
    ArduinoOTA.handle();
  }
}

// Check if OTA is in progress
bool OTAService::isUpdating() const {
  return _updating;
}

// Get OTA progress percentage
uint8_t OTAService::getProgress() const {
  return _progress;
}

// Get OTA status message
String OTAService::getStatus() const {
  return _status;
}

// Setup OTA callbacks
void OTAService::setupOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = F("sketch");
    } else {
      type = F("filesystem");
    }
    
    otaService._updating = true;
    otaService._progress = 0;
    otaService._status = F("OTA Start: ") + type;
    DEBUG_PRINTLN(otaService._status);
  });
  
  ArduinoOTA.onEnd([]() {
    otaService._updating = false;
    otaService._status = F("OTA End");
    DEBUG_PRINTLN(F("OTA End"));
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaService._progress = (progress / (total / 100));
    otaService._status = F("OTA Progress: ") + String(otaService._progress) + F("%");
    DEBUG_PRINT("OTA Progress: ");
    DEBUG_PRINT(otaService._progress);
    DEBUG_PRINTLN(F("%"));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    otaService._updating = false;
    otaService._status = F("OTA Error: ") + String(error);
    DEBUG_PRINT("OTA Error: ");
    DEBUG_PRINTLN(error);
  });
}
