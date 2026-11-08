# Implementation Roadmap

## Phase 1: Core Infrastructure
**Files:** main.cpp, config.h, types.h, sensor_service.cpp/h, led_service.cpp/h, watchdog_service.cpp/h

- [ ] Set up PlatformIO project with required libraries
- [ ] Create `config.h` with pin definitions (GPIO4, GPIO5) and constants
- [ ] Create `types.h` with SensorReading, LEDCommand, Config structs
- [ ] Initialize serial logging in main.cpp
- [ ] Implement sensor_service: DHT22 initialization and reading
- [ ] Implement led_service: FastLED initialization, basic LED control
- [ ] Implement watchdog_service: millis() overflow monitoring
- [ ] Implement basic boot sequence in main.cpp with LED status indication

**Libraries to add to platformio.ini:**
```ini
lib_deps =
  adafruit/DHT sensor library
  fastled/FastLED
  knolleary/PubSubClient
  me-no-dev/ESPAsyncWebServer
  me-no-dev/ESPAsyncTCP
  me-no-dev/AsyncTCP
```

## Phase 2: WiFi and Portal Mode
**Files:** wifi_service.cpp/h, portal_service.cpp/h, buffer_service.cpp/h

- [ ] Implement wifi_service: connection management, AP mode, retry logic (10 attempts)
- [ ] Implement portal_service: DNSServer for captive portal
- [ ] Create web interface for configuration in portal_service
  - [ ] WiFi SSID/Password fields
  - [ ] MQTT broker host/port/credentials
  - [ ] MQTT topic names (sensor, LED)
  - [ ] Display backup credentials if present in EEPROM
  - [ ] Save and apply button with form handling
- [ ] Implement EEPROM storage for backup credentials in portal_service
- [ ] Handle form submission: save config, trigger reboot with new config

## Phase 3: MQTT Mode
**Files:** mqtt_service.cpp/h, sensor_service.cpp/h, led_service.cpp/h

- [ ] Initialize MQTT client with PubSubClient in mqtt_service
- [ ] Implement MQTT connection logic and state management
- [ ] Integrate sensor_service to get readings for publishing
- [ ] Publish sensor data to MQTT topic with retain=true
- [ ] Subscribe to LED control topic
- [ ] Parse and apply LED control commands via led_service
- [ ] Implement MQTT connection monitoring and retry logic
- [ ] Handle MQTT disconnect -> Autonomous Mode transition

## Phase 4: Autonomous Mode
**Files:** buffer_service.cpp/h, api_service.cpp/h, mqtt_service.cpp/h

- [ ] Implement buffer_service: circular buffer of 1000 SensorReading structs
- [ ] Store sensor readings in buffer when MQTT unavailable
- [ ] Implement api_service: HTTP API server (ESPAsyncWebServer)
  - [ ] GET /time - current millis()
  - [ ] GET /current - latest reading
  - [ ] GET /data - full buffer as JSON
  - [ ] GET /info - device status
  - [ ] POST /led - control LEDs
  - [ ] POST /reboot - normal reboot
  - [ ] POST /reboot-clean - reboot to portal, save backup to EEPROM
  - [ ] POST /ota - trigger OTA mode
- [ ] Integrate mqtt_service: periodic reconnection attempts (every 5 minutes)
- [ ] Handle transition back to MQTT Mode on successful reconnection

## Phase 5: Special Features
**Files:** led_service.cpp/h, ota_service.cpp/h, watchdog_service.cpp/h

- [ ] Enhance led_service with status indication patterns
  - [ ] Boot animation (chasing pattern)
  - [ ] Portal mode indicator (slow pulse)
  - [ ] MQTT mode indicator (solid green)
  - [ ] Autonomous mode indicator (solid yellow)
  - [ ] Error/Warning patterns
- [ ] Implement ota_service: OTA update functionality (ArduinoOTA on port 8266)
- [ ] Finalize watchdog_service: millis() overflow protection at 48 days

## Phase 6: Testing and Polish
- [ ] Test WiFi connection and reconnection
- [ ] Test MQTT publishing and subscribing
- [ ] Test portal mode configuration
- [ ] Test autonomous mode with API endpoints
- [ ] Test rolling buffer behavior (overflow, retrieval)
- [ ] Test LED control via MQTT and API
- [ ] Test /reboot and /reboot-clean endpoints
- [ ] Test OTA updates
- [ ] Test millis() overflow protection (simulated)
- [ ] Test backup credentials display in portal

## File Structure

The code is organized into **modular service files** for better separation of concerns.

```
ESP8266 MQTT Node - V2/
├── platformio.ini
├── src/
│   ├── main.cpp              # Application entry, setup(), loop()
│   ├── wifi_service.cpp      # WiFi connection, AP mode, retry logic
│   ├── mqtt_service.cpp      # MQTT client, publishing, subscribing
│   ├── portal_service.cpp    # Config portal web UI, form handling
│   ├── api_service.cpp       # HTTP API endpoints (GET/POST)
│   ├── watchdog_service.cpp  # Millis overflow monitoring
│   ├── led_service.cpp       # SK6812 LED control, status patterns
│   ├── sensor_service.cpp    # DHT22 reading logic
│   ├── buffer_service.cpp    # Rolling buffer management
│   └── ota_service.cpp       # OTA update handling
├── include/
│   ├── config.h              # Constants, pin definitions, defaults
│   ├── types.h               # Shared structs: SensorReading, LEDCommand, Config
│   ├── wifi_service.h
│   ├── mqtt_service.h
│   ├── portal_service.h
│   ├── api_service.h
│   ├── watchdog_service.h
│   ├── led_service.h
│   ├── sensor_service.h
│   ├── buffer_service.h
│   └── ota_service.h
├── lib/
├── docs/
│   └── adr/
│       ├── 0001-use-async-web-server.md
│       ├── 0002-float-for-sensor-readings.md
│       ├── 0003-captive-portal-for-configuration.md
│       ├── 0004-fixed-buffer-size-1000.md
│       ├── 0005-eeprom-for-backup-credentials.md
│       └── 0006-fastled-for-sk6812.md
└── specs/
    ├── SYSTEM_SPECIFICATION.md
    └── IMPLEMENTATION_ROADMAP.md
```

### Service Responsibilities

| Service | Key Functions |
|---------|---------------|
| **wifi_service** | `connectWiFi()`, `startAPMode()`, `retryConnection()` |
| **mqtt_service** | `connectMQTT()`, `publishReading()`, `subscribeToTopics()`, `handleLEDCommand()` |
| **portal_service** | `startPortal()`, `handleConfigForm()`, `servePortalPage()` |
| **api_service** | `handleTime()`, `handleCurrent()`, `handleData()`, `handleLED()`, `handleReboot()` |
| **watchdog_service** | `checkMillisOverflow()`, `triggerReboot()` |
| **led_service** | `setLEDColor()`, `setLEDBrightness()`, `showStatusPattern()`, `updateLEDs()` |
| **sensor_service** | `readSensor()`, `getLastReading()` |
| **buffer_service** | `addReading()`, `getReadings()`, `getLatest()`, `clearBuffer()` |
| **ota_service** | `startOTA()`, `handleOTA()` |

## Estimated Complexity

| Component | Complexity | Notes |
|-----------|------------|-------|
| Hardware initialization | Low | Standard library usage |
| WiFi management | Medium | Retry logic, AP mode |
| Portal mode web UI | Medium | HTML/CSS in C strings, form handling |
| MQTT client | Medium | Connection state management |
| Rolling buffer | Low | Circular buffer pattern |
| HTTP API | Medium | Route handling, JSON serialization |
| LED control | Low | FastLED usage |
| OTA updates | Low | ArduinoOTA library |
| Millis overflow | Low | Simple timer check |

**Total**: Medium complexity project, suitable for intermediate ESP8266 developers.

## Dependencies Summary

| Component | Library | Purpose |
|-----------|---------|---------|
| WiFi | ESP8266WiFi | WiFi connectivity |
| Web Server | ESPAsyncWebServer + AsyncTCP | HTTP API and portal |
| DNS | DNSServer | Captive portal DNS redirection |
| MQTT | PubSubClient | MQTT client |
| DHT22 | Adafruit DHT | Temperature/humidity reading |
| SK6812 | FastLED | RGBW LED control |
| OTA | ArduinoOTA | Firmware updates |
| EEPROM | EEPROM | Backup credentials storage |
