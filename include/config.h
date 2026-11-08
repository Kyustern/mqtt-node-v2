#pragma once

// =============================================================================
// PIN DEFINITIONS
// =============================================================================

// DHT22 Sensor
#define DHT_PIN          D1  // GPIO5
#define DHT_TYPE         DHT22

// SK6812 RGBW LED Strip
#define LED_PIN          D2  // GPIO4
#define LED_COUNT        20
#define LED_COLOR_ORDER  GRBW  // SK6812 is GRBW

// =============================================================================
// SENSOR CONFIGURATION
// =============================================================================

#define MEASUREMENT_INTERVAL_MS  30000  // 30 seconds

// =============================================================================
// NETWORK CONFIGURATION
// =============================================================================

// Default values (can be overridden via portal)
#define DEFAULT_AP_SSID          "ESP8266-Config"
#define DEFAULT_AP_PASSWORD      ""
#define DEFAULT_AP_IP            IPAddress(192, 168, 4, 1)
#define DEFAULT_AP_GATEWAY       IPAddress(192, 168, 4, 1)
#define DEFAULT_AP_SUBNET        IPAddress(255, 255, 255, 0)

#define MQTT_DEFAULT_PORT        1883
#define MQTT_DEFAULT_SENSOR_TOPIC "esp8266/sensor/data"
#define MQTT_DEFAULT_LED_TOPIC   "esp8266/led/set"

// DNS Server for captive portal
#define DNS_PORT                 53

// =============================================================================
// ROLLING BUFFER CONFIGURATION
// =============================================================================

#define BUFFER_CAPACITY          1000

// =============================================================================
// WIFI CONFIGURATION
// =============================================================================

#define WIFI_CONNECTION_RETRIES  10
#define WIFI_RETRY_DELAY_MS      5000  // 5 seconds between retries

// =============================================================================
// MQTT CONFIGURATION
// =============================================================================

#define MQTT_RECONNECT_DELAY_MS   30000  // 30 seconds initial retry
#define MQTT_MAX_RECONNECT_ATTEMPTS 6    // ~30s total with backoff
#define MQTT_AUTONOMOUS_RETRY_MS  300000 // 5 minutes for autonomous mode

// =============================================================================
// WATCHDOG CONFIGURATION
// =============================================================================

// Reboot at 48 days to prevent millis() overflow (50 days = 4294967295ms)
#define MILLIS_OVERFLOW_THRESHOLD  (48UL * 24 * 60 * 60 * 1000)  // 48 days in ms
#define WATCHDOG_CHECK_INTERVAL_MS  3600000  // Check every hour

// =============================================================================
// EEPROM CONFIGURATION
// =============================================================================

#define EEPROM_BACKUP_ADDRESS     0
#define EEPROM_MAX_SIZE           4096  // ESP8266 EEPROM size

// =============================================================================
// LED STATUS INDICATORS
// =============================================================================

#define LED_BOOT_COLOR           CRGB::White
#define LED_PORTAL_COLOR         CRGB::Blue
#define LED_MQTT_COLOR           CRGB::Green
#define LED_AUTONOMOUS_COLOR     CRGB::Yellow
#define LED_ERROR_COLOR          CRGB::Red

#define LED_BOOT_ANIMATION_SPEED  50
#define LED_PULSE_SPEED           20

// =============================================================================
// SERIAL CONFIGURATION
// =============================================================================

#define SERIAL_BAUD_RATE          115200

// =============================================================================
// OTA CONFIGURATION
// =============================================================================

#define OTA_PORT                  8266
#define OTA_HOSTNAME              "ESP8266-MQTT-Node"

// =============================================================================
// DEBUG CONFIGURATION
// Note: DHT sensor library defines DEBUG_PRINT, so we use different names
// =============================================================================

#define DEBUG_ENABLED             true

#if DEBUG_ENABLED
  #define SERIAL_DEBUG(x)         Serial.print(x)
  #define SERIAL_DEBUGLN(x)       Serial.println(x)
  #define SERIAL_DEBUGF(format, args...) Serial.printf(format, ##args)
#else
  #define SERIAL_DEBUG(x)
  #define SERIAL_DEBUGLN(x)
  #define SERIAL_DEBUGF(format, args...)
#endif

// For compatibility with existing code, alias if not already defined
#ifndef DEBUG_PRINT
  #define DEBUG_PRINT(x)         SERIAL_DEBUG(x)
  #define DEBUG_PRINTLN(x)       SERIAL_DEBUGLN(x)
  #define DEBUG_PRINTF(format, ...) SERIAL_DEBUGF(format, ##__VA_ARGS__)
#endif
