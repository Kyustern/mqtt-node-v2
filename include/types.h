#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

// =============================================================================
// SENSOR READING STRUCT
// =============================================================================

struct SensorReading {
  unsigned long time;      // millis() timestamp
  float temperature;        // °C
  float humidity;           // % RH
  
  // Default constructor
  SensorReading() : time(0), temperature(0.0), humidity(0.0) {}
  
  // Parameterized constructor
  SensorReading(unsigned long t, float temp, float hum) 
    : time(t), temperature(temp), humidity(hum) {}
  
  // Check if reading is valid
  bool isValid() const {
    return temperature >= -50 && temperature <= 100 && 
           humidity >= 0 && humidity <= 100;
  }
};

// =============================================================================
// LED COMMAND STRUCT
// =============================================================================

struct LEDCommand {
  uint8_t r;        // Red channel (0-255)
  uint8_t g;        // Green channel (0-255)
  uint8_t b;        // Blue channel (0-255)
  uint8_t w;        // White channel (0-255)
  uint8_t brightness; // Global brightness (0-100)
  bool enabled;     // Whether LEDs are enabled
  
  // Default constructor - all off
  LEDCommand() : r(0), g(0), b(0), w(0), brightness(100), enabled(false) {}
  
  // Convert to CRGB for FastLED
  CRGB toCRGB() const {
    // Apply brightness scaling
    float scale = brightness / 100.0;
    return CRGB(
      static_cast<uint8_t>(r * scale),
      static_cast<uint8_t>(g * scale),
      static_cast<uint8_t>(b * scale)
    );
  }
  
  // Check if command is valid
  bool isValid() const {
    return r <= 255 && g <= 255 && b <= 255 && w <= 255 && 
           brightness <= 100;
  }
};

// =============================================================================
// WIFI CONFIGURATION STRUCT
// =============================================================================

struct WiFiConfig {
  String ssid;
  String password;
  
  bool isValid() const {
    return !ssid.isEmpty();
  }
  
  void clear() {
    ssid = "";
    password = "";
  }
};

// =============================================================================
// MQTT CONFIGURATION STRUCT
// =============================================================================

struct MQTTConfig {
  String host;
  uint16_t port;
  String username;
  String password;
  String sensor_topic;
  String led_topic;
  
  MQTTConfig() : port(MQTT_DEFAULT_PORT) {}
  
  // Check if configuration is valid (minimal: host and port)
  bool isValid() const {
    return !host.isEmpty() && port > 0;
  }
  
  // Check if all MQTT fields are populated (optional: all must be filled to attempt MQTT)
  bool isComplete() const {
    return !host.isEmpty() && 
           port > 0 && 
           !sensor_topic.isEmpty() && 
           !led_topic.isEmpty();
  }
  
  void clear() {
    host = "";
    port = MQTT_DEFAULT_PORT;
    username = "";
    password = "";
    sensor_topic = "";
    led_topic = "";
  }
};

// =============================================================================
// DEVICE CONFIGURATION STRUCT
// =============================================================================

struct DeviceConfig {
  WiFiConfig wifi;
  MQTTConfig mqtt;
  
  bool isValid() const {
    return wifi.isValid();
  }
  
  void clear() {
    wifi.clear();
    mqtt.clear();
  }
};

// =============================================================================
// DEVICE STATUS ENUM
// =============================================================================

enum class DeviceMode {
  UNKNOWN,
  PORTAL,      // Configuration mode (AP mode)
  MQTT,        // Connected to WiFi and MQTT
  AUTONOMOUS   // Connected to WiFi but not MQTT
};

// Convert DeviceMode to string
inline const char* deviceModeToString(DeviceMode mode) {
  switch (mode) {
    case DeviceMode::PORTAL: return "portal";
    case DeviceMode::MQTT: return "mqtt";
    case DeviceMode::AUTONOMOUS: return "autonomous";
    default: return "unknown";
  }
}

// =============================================================================
// DEVICE INFO STRUCT (for API responses)
// =============================================================================

struct DeviceInfo {
  String device_name;
  DeviceMode mode;
  unsigned long uptime;
  uint32_t free_heap;
  String ip_address;
  int8_t wifi_rssi;
};

// =============================================================================
// BUFFER INDEX TYPE
// =============================================================================

using BufferIndex = uint16_t;  // Max 65535, more than enough for BUFFER_CAPACITY
