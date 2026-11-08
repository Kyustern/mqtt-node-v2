#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "types.h"
#include "wifi_service.h"
#include "mqtt_service.h"
#include "portal_service.h"
#include "api_service.h"
#include "watchdog_service.h"
#include "led_service.h"
#include "sensor_service.h"
#include "buffer_service.h"
#include "ota_service.h"

// =============================================================================
// GLOBAL STATE
// =============================================================================

DeviceMode currentMode = DeviceMode::UNKNOWN;
DeviceConfig currentConfig;

// =============================================================================
// CALLBACK FUNCTIONS
// =============================================================================

// Callback for LED commands from MQTT
void handleLEDCommand(const String& payload) {
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, payload);
  
  if (!error) {
    LEDCommand command;
    if (doc.containsKey("r")) command.r = doc["r"];
    if (doc.containsKey("g")) command.g = doc["g"];
    if (doc.containsKey("b")) command.b = doc["b"];
    if (doc.containsKey("w")) command.w = doc["w"];
    if (doc.containsKey("brightness")) command.brightness = doc["brightness"];
    if (doc.containsKey("enabled")) command.enabled = doc["enabled"];
    
    if (command.isValid()) {
      ledService.applyCommand(command);
      DEBUG_PRINTLN(F("LED command applied from MQTT"));
    }
  }
}

// Callback for reboot
void handleReboot() {
  DEBUG_PRINTLN(F("Rebooting..."));
  delay(100);
  ESP.restart();
}

// Callback for clean reboot
void handleRebootClean() {
  DEBUG_PRINTLN(F("Saving backup and rebooting to Portal Mode..."));
  portalService.saveBackupConfig(currentConfig);
  delay(100);
  ESP.restart();
}

// Callback for OTA
void handleOTA() {
  DEBUG_PRINTLN(F("OTA requested"));
  // OTA is already running in the background
}

// Get device info
DeviceInfo getDeviceInfo() {
  DeviceInfo info;
  info.device_name = F("ESP8266 MQTT Node");
  info.mode = currentMode;
  info.uptime = millis() / 1000;
  info.free_heap = ESP.getFreeHeap();
  info.ip_address = wifiService.getLocalIP().toString();
  info.wifi_rssi = wifiService.getRSSI();
  return info;
}

// =============================================================================
// MODE MANAGEMENT
// =============================================================================

void enterPortalMode() {
  DEBUG_PRINTLN(F("\n[MODE] ===== ENTERING PORTAL MODE ====="));
  currentMode = DeviceMode::PORTAL;
  mqttService.setMode(DeviceMode::PORTAL);
  mqttService.disconnect();
  
  wifiService.startAPMode();
  portalService.startPortal();
  apiService.stopServer();
  
  ledService.showStatusPattern(DeviceMode::PORTAL);
  DEBUG_PRINTLN(F("[MODE] Portal mode active\n"));
}

void enterMQTTMode() {
  DEBUG_PRINTLN(F("\n[MODE] ===== ENTERING MQTT MODE ====="));
  currentMode = DeviceMode::MQTT;
  mqttService.setMode(DeviceMode::MQTT);
  
  portalService.stopPortal();
  apiService.startServer();
  
  ledService.showStatusPattern(DeviceMode::MQTT);
  DEBUG_PRINTLN(F("[MODE] MQTT mode active\n"));
}

void enterAutonomousMode() {
  DEBUG_PRINTLN(F("\n[MODE] ===== ENTERING AUTONOMOUS MODE ====="));
  currentMode = DeviceMode::AUTONOMOUS;
  mqttService.setMode(DeviceMode::AUTONOMOUS);
  
  portalService.stopPortal();
  apiService.startServer();
  
  ledService.showStatusPattern(DeviceMode::AUTONOMOUS);
  DEBUG_PRINTLN(F("[MODE] Autonomous mode active - Will retry MQTT periodically\n"));
}

// =============================================================================
// CONFIGURATION MANAGEMENT
// =============================================================================

void loadConfiguration() {
  DEBUG_PRINTLN(F("\n[CONFIG] ===== LOADING CONFIGURATION ====="));
  portalService.loadBackupConfig(currentConfig);
  
  if (!currentConfig.isValid()) {
    DEBUG_PRINTLN(F("[CONFIG] No valid configuration found in backup"));
    currentConfig.clear();
  } else {
    DEBUG_PRINTLN(F("[CONFIG] Configuration loaded from backup"));
    DEBUG_PRINT(F("[CONFIG] WiFi SSID: "));
    DEBUG_PRINTLN(currentConfig.wifi.ssid);
    DEBUG_PRINT(F("[CONFIG] WiFi Password: "));
    DEBUG_PRINT(currentConfig.wifi.password.length());
    DEBUG_PRINTLN(F(" chars"));
    DEBUG_PRINT(F("[CONFIG] MQTT Host: "));
    DEBUG_PRINTLN(currentConfig.mqtt.host);
    DEBUG_PRINT(F("[CONFIG] MQTT Port: "));
    DEBUG_PRINTLN(currentConfig.mqtt.port);
    DEBUG_PRINT(F("[CONFIG] Sensor Topic: "));
    DEBUG_PRINTLN(currentConfig.mqtt.sensor_topic);
    DEBUG_PRINT(F("[CONFIG] LED Topic: "));
    DEBUG_PRINTLN(currentConfig.mqtt.led_topic);
  }
  DEBUG_PRINTLN(F("[CONFIG] ===== CONFIGURATION LOADED =====\n"));
}

void saveConfiguration() {
  portalService.saveBackupConfig(currentConfig);
}

// =============================================================================
// SENSOR READING
// =============================================================================

void takeReading() {
  SensorReading reading = sensorService.read();
  
  if (reading.isValid()) {
    // Store in buffer regardless of mode
    bufferService.addReading(reading);
    
    // Publish if in MQTT mode
    if (currentMode == DeviceMode::MQTT && mqttService.isConnected()) {
      mqttService.publishReading(reading);
    }
  } else {
    DEBUG_PRINTLN(F("Invalid sensor reading"));
  }
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  // Initialize serial first
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Wait for serial to be ready
  unsigned long startTime = millis();
  while (!Serial && millis() - startTime < 5000) {
    delay(10);
  }
  
  // Blink built-in LED to show we're alive
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  
  DEBUG_PRINTLN(F("\n\n========================================"));
  DEBUG_PRINTLN(F("=== ESP8266 MQTT Node ==="));
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTLN(F("[SYSTEM] Initializing...\n"));
  
  // Initialize services
  DEBUG_PRINTLN(F("[SYSTEM] Initializing services..."));
  ledService.begin();
  sensorService.begin();
  bufferService.begin();
  watchdogService.begin();
  wifiService.begin();
  otaService.begin(OTA_HOSTNAME);
  DEBUG_PRINTLN(F("[SYSTEM] Services initialized\n"));
  
  // Initialize API service with dependencies
  apiService.begin(bufferService, ledService);
  
  // Initialize portal service
  portalService.begin();
  
  // Initialize MQTT service
  mqttService.begin();
  
  // Set callbacks
  mqttService.setLEDCommandCallback(handleLEDCommand);
  apiService.setRebootCallback(handleReboot);
  apiService.setRebootCleanCallback(handleRebootClean);
  apiService.setOTACallback(handleOTA);
  apiService.setDeviceInfoCallback(getDeviceInfo);
  
  DEBUG_PRINTLN(F("[SYSTEM] Callbacks set\n"));
  
  // Show boot animation
  ledService.showBootAnimation();
  
  // Load configuration
  loadConfiguration();
  
  // Try to connect based on configuration
  if (!currentConfig.isValid()) {
    DEBUG_PRINTLN(F("[BOOT] No configuration found. Entering Portal Mode."));
    enterPortalMode();
    return;
  }
  
  // Try WiFi connection
  DEBUG_PRINTLN(F("[BOOT] WiFi configuration found. Attempting connection..."));
  if (!wifiService.connect(currentConfig.wifi)) {
    DEBUG_PRINTLN(F("[BOOT] ERROR: WiFi connection failed. Entering Portal Mode."));
    enterPortalMode();
    return;
  }
  
  DEBUG_PRINTLN(F("[BOOT] WiFi connected successfully!"));
  
  // Check if MQTT configuration is complete
  bool mqttConfigComplete = currentConfig.mqtt.isComplete();
  
  if (mqttConfigComplete) {
    DEBUG_PRINTLN(F("[BOOT] MQTT configuration is complete. Attempting MQTT connection..."));
    
    // Configure MQTT
    mqttService.setConfig(currentConfig.mqtt);
    
    // Try MQTT connection
    if (mqttService.connect(currentConfig.mqtt)) {
      DEBUG_PRINTLN(F("[BOOT] MQTT connected successfully. Entering MQTT Mode."));
      enterMQTTMode();
    } else {
      DEBUG_PRINTLN(F("[BOOT] WARNING: MQTT connection failed. Entering Autonomous Mode."));
      enterAutonomousMode();
    }
  } else {
    DEBUG_PRINTLN(F("[BOOT] MQTT configuration incomplete. Missing fields:"));
    if (currentConfig.mqtt.host.isEmpty()) DEBUG_PRINTLN(F("  - Host"));
    if (currentConfig.mqtt.port == 0) DEBUG_PRINTLN(F("  - Port"));
    if (currentConfig.mqtt.sensor_topic.isEmpty()) DEBUG_PRINTLN(F("  - Sensor topic"));
    if (currentConfig.mqtt.led_topic.isEmpty()) DEBUG_PRINTLN(F("  - LED topic"));
    DEBUG_PRINTLN(F("[BOOT] Skipping MQTT, entering Autonomous Mode."));
    enterAutonomousMode();
  }
  
  // Start OTA server
  otaService.start();
  
  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F("[SYSTEM] Setup complete!"));
  DEBUG_PRINT(F("[SYSTEM] Current mode: "));
  DEBUG_PRINTLN(deviceModeToString(currentMode));
  DEBUG_PRINTLN(F("========================================\n"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  // Blink built-in LED every 2 seconds as heartbeat
  static unsigned long lastBlink = 0;
  static unsigned long lastStatus = 0;
  
  if (millis() - lastBlink >= 2000) {
    lastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  
  // Print status periodically
  if (millis() - lastStatus >= 10000) {
    lastStatus = millis();
    DEBUG_PRINTLN(F("\n[STATUS] ===== PERIODIC STATUS ====="));
    DEBUG_PRINT(F("[STATUS] Mode: "));
    DEBUG_PRINTLN(deviceModeToString(currentMode));
    DEBUG_PRINT(F("[STATUS] Uptime: "));
    DEBUG_PRINT(millis() / 1000);
    DEBUG_PRINTLN(F(" seconds"));
    DEBUG_PRINT(F("[STATUS] Free heap: "));
    DEBUG_PRINT(ESP.getFreeHeap());
    DEBUG_PRINTLN(F(" bytes"));
    DEBUG_PRINT(F("[STATUS] WiFi connected: "));
    DEBUG_PRINTLN(wifiService.isConnected() ? "Yes" : "No");
    if (wifiService.isConnected()) {
      DEBUG_PRINT(F("[STATUS] IP: "));
      DEBUG_PRINTLN(wifiService.getLocalIP());
      DEBUG_PRINT(F("[STATUS] RSSI: "));
      DEBUG_PRINT(wifiService.getRSSI());
      DEBUG_PRINTLN(F(" dBm"));
    }
    DEBUG_PRINT(F("[STATUS] MQTT connected: "));
    DEBUG_PRINTLN(mqttService.isConnected() ? "Yes" : "No");
    DEBUG_PRINT(F("[STATUS] Buffer entries: "));
    DEBUG_PRINT(bufferService.count());
    DEBUG_PRINTLN(F("/1000"));
    DEBUG_PRINTLN(F("[STATUS] ================================\n"));
  }
  
  // Process services
  watchdogService.check();
  otaService.loop();
  
  // Process mode-specific services
  switch (currentMode) {
    case DeviceMode::PORTAL:
      portalService.loop();
      ledService.loop();
      break;
      
    case DeviceMode::MQTT:
      mqttService.loop();
      ledService.loop();
      
      // Take sensor readings periodically
      static unsigned long lastReadingTime = 0;
      if (millis() - lastReadingTime >= MEASUREMENT_INTERVAL_MS) {
        lastReadingTime = millis();
        takeReading();
      }
      break;
      
    case DeviceMode::AUTONOMOUS:
      mqttService.loop();
      apiService.loop();
      ledService.loop();
      
      // Take sensor readings periodically
      if (millis() - lastReadingTime >= MEASUREMENT_INTERVAL_MS) {
        lastReadingTime = millis();
        takeReading();
      }
      break;
      
    default:
      break;
  }
  
  // Small delay to prevent watchdog reset
  delay(10);
}
