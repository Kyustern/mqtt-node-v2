# ESP8266 MQTT Node - System Specification

## Overview

An ESP8266-based IoT device (Wemos D1 mini) that collects temperature and humidity data using a DHT22 sensor, controls an SK6812 RGBW LED strip, and exposes data over a network via MQTT and HTTP API.

## Hardware

| Component | Model | Connection | Purpose |
|-----------|-------|-------------|---------|
| Microcontroller | Wemos D1 mini (ESP8266) | - | Core processing |
| Temperature/Humidity Sensor | DHT22 | GPIO5 (D1) | Environmental sensing |
| RGBW LED Strip | SK6812 | GPIO4 (D2) | Status indication & user control |

- **LED Strip**: 20 LEDs, individually addressable RGBW

## Operating Modes

### 1. Portal Mode
**Entry Condition**: No WiFi configuration or WiFi connection fails after 10 retries.

**Behavior**:
- Activates built-in access point with captive portal
- IP address: `192.168.4.1`
- All DNS requests redirect to portal (captive portal)
- Serves web interface for configuration
- Displays backup credentials if present in EEPROM

**Configuration Parameters** (stored in persistent memory):
- WiFi SSID
- WiFi Password
- MQTT Broker Host
- MQTT Broker Port (default: 1883)
- MQTT Username (optional)
- MQTT Password (optional)
- MQTT Sensor Data Topic
- MQTT LED Control Topic

### 2. MQTT Mode
**Entry Condition**: Successful WiFi connection AND successful MQTT broker connection.

**Behavior**:
- Takes temperature/humidity measurements every 30 seconds
- Publishes sensor data to configured MQTT topic with retain flag = true
- Subscribes to LED control MQTT topic
- Processes incoming LED control commands
- Monitors MQTT connection health

**MQTT Message Formats**:

**Sensor Data (published)**:
```json
{
  "time": 12345678,
  "temperature": 23.5,
  "humidity": 45.2
}
```

**LED Control (subscribed)**:
```json
{
  "r": 255,
  "g": 0,
  "b": 0,
  "w": 0,
  "brightness": 100,
  "enabled": true
}
```

### 3. Autonomous Mode
**Entry Condition**: WiFi connected but MQTT broker connection fails.

**Behavior**:
- Takes temperature/humidity measurements every 30 seconds
- Stores readings in rolling buffer (1000 entries maximum)
- Exposes HTTP API endpoints
- Periodically attempts MQTT reconnection (every 5 minutes)
- If MQTT reconnects successfully, transitions to MQTT Mode

**Transition Logic**:
- On boot: Portal Mode → (WiFi success) → MQTT Mode attempt → (MQTT success) → MQTT Mode OR (MQTT fail) → Autonomous Mode
- WiFi disconnect: Immediately attempt reconnection (10 retries) → Portal Mode if all fail
- MQTT disconnect in MQTT Mode: Retry for ~30s with exponential backoff → Autonomous Mode
- In Autonomous Mode: Retry MQTT connection every 5 minutes

## Data Structures

### Sensor Reading Struct
```cpp
struct SensorReading {
  unsigned long time;   // millis() timestamp (4 bytes)
  float temperature;     // °C (4 bytes)
  float humidity;        // % RH (4 bytes)
};
// Total: 12 bytes per reading
```

### Rolling Buffer
- **Type**: Circular buffer / Fixed-size array
- **Capacity**: 1000 readings
- **Memory**: 12,000 bytes (12 KB)
- **Behavior**: New readings overwrite oldest entries

## HTTP API (Available in Autonomous Mode and MQTT Mode)

All endpoints run on port 80.

### GET Endpoints

| Endpoint | Response | Description |
|----------|----------|-------------|
| `/time` | `{"millis": 12345678}` | Current millis() value |
| `/current` | `{"time": 12345678, "temperature": 23.5, "humidity": 45.2}` | Latest sensor reading |
| `/data` | `[{"time":..., "temperature":..., "humidity":...}, ...]` | Full rolling buffer as JSON array |
| `/info` | `{"device": "ESP8266 MQTT Node", "mode": "autonomous", "uptime": 3600, "free_heap": 20480, "ip": "192.168.1.100"}` | Device information |

### POST Endpoints

| Endpoint | Request Body | Response | Description |
|----------|--------------|----------|-------------|
| `/led` | `{"r":255,"g":0,"b":0,"w":0,"brightness":100,"enabled":true}` | `{"status": "ok"}` | Set LED strip color and state |
| `/reboot` | - | `{"status": "rebooting"}` | Reboot device with current configuration |
| `/reboot-clean` | - | `{"status": "rebooting"}` | Reboot to Portal Mode, save current credentials to EEPROM backup |
| `/ota` | - | `{"status": "ota_ready"}` | Trigger OTA update mode (port 8266) |

## Persistent Storage

### EEPROM Usage
- **Purpose**: Store backup credentials when `/reboot-clean` is called
- **Format**: JSON string
- **Location**: EEPROM address 0
- **Backup JSON Schema**:
```json
{
  "wifi": {
    "ssid": "...",
    "password": "..."
  },
  "mqtt": {
    "host": "...",
    "port": 1883,
    "username": "...",
    "password": "...",
    "sensor_topic": "...",
    "led_topic": "..."
  }
}
```
- **Portal Behavior**: If EEPROM contains valid backup data, display it as "Previous Configuration" on the web portal

## Special Behaviors

### Millis Overflow Protection
- **Trigger**: At approximately 48 days of continuous uptime
- **Action**: Automatically reboot using the same routine as `/reboot` endpoint
- **Purpose**: Prevent millis() overflow (occurs at 49.7 days)

### LED Status Indication
- **Boot**: LEDs show boot animation (e.g., chasing pattern)
- **Portal Mode**: LEDs display WiFi AP mode indicator (e.g., slow pulse)
- **MQTT Mode**: LEDs show connected status (e.g., solid color)
- **Autonomous Mode**: LEDs show fallback status (e.g., different solid color)

## Libraries

| Purpose | Library | Notes |
|---------|---------|-------|
| WiFi | ESP8266WiFi | Built-in |
| Web Server | ESPAsyncWebServer | Async for better performance |
| DNS Server | DNSServer | For captive portal |
| MQTT Client | PubSubClient | Standard MQTT library |
| DHT22 Sensor | DHT sensor library by Adafruit | For DHT22 |
| SK6812 LEDs | FastLED | Supports RGBW LEDs |
| OTA | ArduinoOTA | Built-in |
| EEPROM | EEPROM | Built-in |

## Network Configuration

- **Web Server Port**: 80
- **OTA Port**: 8266
- **MQTT Default Port**: 1883
- **AP IP Address**: 192.168.4.1
- **AP SSID**: "ESP8266-Config" (or configurable)

## Memory Constraints

- **ESP8266 Heap**: ~40-50 KB available
- **Rolling Buffer**: 12 KB (1000 readings × 12 bytes)
- **Remaining Heap**: ~28-38 KB for other operations (sufficient)

## JSON Schemas

### Sensor Reading
```json
{
  "time": <unsigned long>,
  "temperature": <float>,
  "humidity": <float>
}
```

### LED Control
```json
{
  "r": <0-255>,
  "g": <0-255>,
  "b": <0-255>,
  "w": <0-255>,
  "brightness": <0-100>,
  "enabled": <boolean>
}
```

### Device Info
```json
{
  "device": "ESP8266 MQTT Node",
  "mode": "mqtt" | "autonomous" | "portal",
  "uptime": <seconds>,
  "free_heap": <bytes>,
  "ip": <string>,
  "wifi_rssi": <dBm>
}
```

## Boot Sequence

1. Initialize hardware (serial, LEDs, sensors)
2. Show boot animation on LEDs
3. Check for WiFi configuration
4. If no config: Enter Portal Mode
5. If config exists: Attempt WiFi connection (10 retries)
6. If WiFi fails: Enter Portal Mode
7. If WiFi succeeds: Attempt MQTT connection
8. If MQTT succeeds: Enter MQTT Mode
9. If MQTT fails: Enter Autonomous Mode
10. Start measurement loop (30s intervals)

## Error Handling

- **WiFi Connection Failure**: Retry up to 10 times with backoff, then Portal Mode
- **MQTT Connection Failure (initial)**: Enter Autonomous Mode
- **MQTT Connection Failure (after success)**: Retry for ~30s with exponential backoff, then Autonomous Mode
- **DHT22 Read Failure**: Log error, use previous reading or skip
- **LED Control Parse Error**: Return 400 Bad Request, maintain current LED state

## Code Architecture

The code is organized into **modular service files**, each with a single responsibility. This separation improves maintainability, testability, and clarity.

### Service Modules

| Service | File | Responsibility |
|---------|------|----------------|
| **Main** | `main.cpp` | Application entry point, initialization, main loop |
| **WiFi Service** | `wifi_service.cpp/h` | WiFi connection management, AP mode, retry logic |
| **MQTT Service** | `mqtt_service.cpp/h` | MQTT client, publishing sensor data, subscribing to LED commands |
| **Portal Service** | `portal_service.cpp/h` | Configuration web portal, form handling, captive portal DNS |
| **API Service** | `api_service.cpp/h` | HTTP API endpoints (GET /time, /current, /data, /info; POST /led, /reboot, /reboot-clean, /ota) |
| **Watchdog Service** | `watchdog_service.cpp/h` | Millis overflow monitoring, 48-day auto-reboot |
| **LED Service** | `led_service.cpp/h` | SK6812 LED control, status indication patterns |
| **Sensor Service** | `sensor_service.cpp/h` | DHT22 measurement, error handling |
| **Buffer Service** | `buffer_service.cpp/h` | Rolling buffer management, add/read operations |
| **OTA Service** | `ota_service.cpp/h` | OTA update handling, port 8266 server |

### Shared Components

| Component | File | Purpose |
|-----------|------|---------|
| **Config** | `config.h` | Compile-time constants, pin definitions |
| **Types** | `types.h` | Shared data structures (SensorReading, LEDCommand, Config, etc.) |

### File Structure

```
ESP8266 MQTT Node - V2/
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── wifi_service.cpp
│   ├── mqtt_service.cpp
│   ├── portal_service.cpp
│   ├── api_service.cpp
│   ├── watchdog_service.cpp
│   ├── led_service.cpp
│   ├── sensor_service.cpp
│   ├── buffer_service.cpp
│   └── ota_service.cpp
├── include/
│   ├── config.h
│   ├── types.h
│   ├── wifi_service.h
│   ├── mqtt_service.h
│   ├── portal_service.h
│   ├── api_service.h
│   ├── watchdog_service.h
│   ├── led_service.h
│   ├── sensor_service.h
│   ├── buffer_service.h
│   └── ota_service.h
└── specs/
    └── ...
```

## Future Considerations

- MQTT QoS configuration (currently default QoS 0)
- MQTT TLS support (currently not implemented)
- Multiple LED strips or zones
- Additional sensors (pressure, CO2, etc.)
- Deep sleep mode for battery operation
- Configuration backup/restore via API
