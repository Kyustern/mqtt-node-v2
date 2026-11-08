# Domain Glossary

This document defines the canonical terminology for the ESP8266 MQTT Node project.

## Core Concepts

**Device**
: The physical hardware unit: a Wemos D1 mini (ESP8266) with a DHT22 sensor and an SK6812 RGBW LED strip attached.

**Sensor Reading**
: A single measurement of environmental conditions captured at a point in time. Contains: timestamp (millis), temperature (°C), humidity (% RH).

**Rolling Buffer**
: A fixed-size circular buffer storing the most recent Sensor Readings. When full, new readings overwrite the oldest. Capacity: 1000 readings.

## Operating Modes

**Portal Mode**
: The configuration mode. The device hosts its own WiFi access point and captive portal, allowing users to set WiFi and MQTT credentials. Activated when no valid configuration exists or WiFi connection fails repeatedly.

**MQTT Mode**
: The primary operational mode. The device is connected to both WiFi and an MQTT broker. It publishes Sensor Readings to the broker and listens for LED control commands.

**Autonomous Mode**
: The fallback operational mode. The device is connected to WiFi but cannot reach the MQTT broker. It continues taking measurements, storing them in the Rolling Buffer, and exposing them via HTTP API. Periodically attempts to reconnect to MQTT.

## Network Components

**MQTT Broker**
: The central message broker that receives Sensor Readings and distributes LED control commands. Not part of this device; external infrastructure.

**MQTT Topic**
: A named channel on the MQTT Broker. This device uses two: one for publishing Sensor Readings, one for subscribing to LED Control Commands.

**HTTP API**
: The REST-like interface exposed by the device on port 80. Available in both MQTT Mode and Autonomous Mode. Provides access to Sensor Readings, device status, and control functions.

**Captive Portal**
: A special HTTP server behavior that redirects all DNS requests to the device's own IP address, making the configuration web interface easily accessible regardless of what URL the user enters.

## Hardware Components

**DHT22**
: The temperature and humidity sensor. Connected to GPIO5. Provides measurements with 0.1°C and 0.1% RH resolution.

**SK6812**
: The RGBW LED strip. 20 LEDs connected to GPIO4. Individually addressable, supports red, green, blue, and white channels.

## Data Structures

**LED Command**
: An instruction to set the state of the SK6812 LED strip. Contains: red, green, blue, white channel values (0-255), brightness (0-100), and enabled flag (boolean). Delivered via MQTT or HTTP API.

**Device Info**
: A snapshot of the device's current state. Contains: mode, uptime, free memory, IP address, WiFi signal strength.

## Special States

**Backup Credentials**
: A copy of the current configuration stored in EEPROM when the user triggers a clean reboot. Preserved across reboots and displayed in Portal Mode to help users remember their previous settings.

**Millis Overflow**
: The condition where the ESP8266's millis() counter exceeds its maximum value (approximately 49.7 days). The device automatically reboots at ~48 days to prevent this.
