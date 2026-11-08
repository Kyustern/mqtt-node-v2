#include "api_service.h"
#include "buffer_service.h"
#include "led_service.h"
#include "config.h"
#include <ArduinoJson.h>

APIService apiService;

// Constructor
APIService::APIService() 
  : _server(80),
    _bufferService(nullptr),
    _ledService(nullptr),
    _running(false),
    _rebootCallback(nullptr),
    _rebootCleanCallback(nullptr),
    _otaCallback(nullptr),
    _deviceInfoCallback(nullptr) {
}

// Initialize with services
void APIService::begin(BufferService& buffer, LEDService& led) {
  _bufferService = &buffer;
  _ledService = &led;
  setupRoutes();
  _running = false;
  DEBUG_PRINTLN(F("API service initialized"));
}

// Start API server
void APIService::startServer() {
  _server.begin();
  _running = true;
  DEBUG_PRINTLN(F("API server started on port 80"));
}

// Stop API server
void APIService::stopServer() {
  _server.stop();
  _running = false;
  DEBUG_PRINTLN(F("API server stopped"));
}

// Process API requests (call in main loop)
void APIService::loop() {
  if (_running) {
    _server.handleClient();
  }
}

// Check if server is running
bool APIService::isRunning() const {
  return _running;
}

// Set callbacks
void APIService::setRebootCallback(void (*callback)()) {
  _rebootCallback = callback;
}

void APIService::setRebootCleanCallback(void (*callback)()) {
  _rebootCleanCallback = callback;
}

void APIService::setOTACallback(void (*callback)()) {
  _otaCallback = callback;
}

void APIService::setDeviceInfoCallback(DeviceInfo (*callback)()) {
  _deviceInfoCallback = callback;
}

// Setup routes
void APIService::setupRoutes() {
  // GET endpoints
  _server.on("/time", HTTP_GET, [this]() { handleTime(); });
  _server.on("/current", HTTP_GET, [this]() { handleCurrent(); });
  _server.on("/data", HTTP_GET, [this]() { handleData(); });
  _server.on("/info", HTTP_GET, [this]() { handleInfo(); });
  
  // POST endpoints
  _server.on("/led", HTTP_POST, [this]() { handleLED(); });
  _server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
  _server.on("/reboot-clean", HTTP_POST, [this]() { handleRebootClean(); });
  _server.on("/ota", HTTP_POST, [this]() { handleOTA(); });
  
  // Enable CORS
  _server.enableCORS(true);
}

// Handle GET /time
void APIService::handleTime() {
  DynamicJsonDocument doc(128);
  doc["millis"] = millis();
  
  String json;
  serializeJson(doc, json);
  sendJsonResponse(200, json);
}

// Handle GET /current
void APIService::handleCurrent() {
  if (_bufferService == nullptr) {
    sendErrorResponse(500, F("Buffer service not initialized"));
    return;
  }
  
  SensorReading reading = _bufferService->getLatest();
  
  if (!reading.isValid()) {
    sendErrorResponse(404, F("No readings available"));
    return;
  }
  
  String json = sensorReadingToJson(reading);
  sendJsonResponse(200, json);
}

// Handle GET /data
void APIService::handleData() {
  if (_bufferService == nullptr) {
    sendErrorResponse(500, F("Buffer service not initialized"));
    return;
  }
  
  String json = _bufferService->toJson();
  sendJsonResponse(200, json);
}

// Handle GET /info
void APIService::handleInfo() {
  if (_deviceInfoCallback == nullptr) {
    sendErrorResponse(500, F("Device info callback not set"));
    return;
  }
  
  DeviceInfo info = _deviceInfoCallback();
  String json = deviceInfoToJson(info);
  sendJsonResponse(200, json);
}

// Handle POST /led
void APIService::handleLED() {
  if (_ledService == nullptr) {
    sendErrorResponse(500, F("LED service not initialized"));
    return;
  }
  
  if (_server.hasArg("plain")) {
    String payload = _server.arg("plain");
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      sendErrorResponse(400, F("Invalid JSON"));
      return;
    }
    
    LEDCommand command;
    if (doc.containsKey("r")) command.r = doc["r"];
    if (doc.containsKey("g")) command.g = doc["g"];
    if (doc.containsKey("b")) command.b = doc["b"];
    if (doc.containsKey("w")) command.w = doc["w"];
    if (doc.containsKey("brightness")) command.brightness = doc["brightness"];
    if (doc.containsKey("enabled")) command.enabled = doc["enabled"];
    
    if (!command.isValid()) {
      sendErrorResponse(400, F("Invalid LED command"));
      return;
    }
    
    _ledService->applyCommand(command);
    
    DynamicJsonDocument response(32);
    response["status"] = "ok";
    String json;
    serializeJson(response, json);
    sendJsonResponse(200, json);
  } else {
    sendErrorResponse(400, F("No payload provided"));
  }
}

// Handle POST /reboot
void APIService::handleReboot() {
  DynamicJsonDocument response(32);
  response["status"] = "rebooting";
  String json;
  serializeJson(response, json);
  sendJsonResponse(200, json);
  
  // Delay to allow response to be sent
  delay(100);
  
  if (_rebootCallback != nullptr) {
    _rebootCallback();
  } else {
    ESP.restart();
  }
}

// Handle POST /reboot-clean
void APIService::handleRebootClean() {
  DynamicJsonDocument response(32);
  response["status"] = "rebooting";
  String json;
  serializeJson(response, json);
  sendJsonResponse(200, json);
  
  // Delay to allow response to be sent
  delay(100);
  
  if (_rebootCleanCallback != nullptr) {
    _rebootCleanCallback();
  } else {
    ESP.restart();
  }
}

// Handle POST /ota
void APIService::handleOTA() {
  DynamicJsonDocument response(32);
  response["status"] = "ota_ready";
  String json;
  serializeJson(response, json);
  sendJsonResponse(200, json);
  
  if (_otaCallback != nullptr) {
    _otaCallback();
  }
}

// Send JSON response
void APIService::sendJsonResponse(int code, const String& json) {
  _server.send(code, "application/json", json);
}

// Send error response
void APIService::sendErrorResponse(int code, const String& message) {
  DynamicJsonDocument doc(128);
  doc["error"] = message;
  
  String json;
  serializeJson(doc, json);
  _server.send(code, "application/json", json);
}

// Convert SensorReading to JSON
String APIService::sensorReadingToJson(const SensorReading& reading) const {
  DynamicJsonDocument doc(128);
  doc["time"] = reading.time;
  doc["temperature"] = reading.temperature;
  doc["humidity"] = reading.humidity;
  
  String json;
  serializeJson(doc, json);
  return json;
}

// Convert DeviceInfo to JSON
String APIService::deviceInfoToJson(const DeviceInfo& info) const {
  DynamicJsonDocument doc(256);
  doc["device"] = info.device_name;
  doc["mode"] = deviceModeToString(info.mode);
  doc["uptime"] = info.uptime;
  doc["free_heap"] = info.free_heap;
  doc["ip"] = info.ip_address;
  doc["wifi_rssi"] = info.wifi_rssi;
  
  String json;
  serializeJson(doc, json);
  return json;
}
