# 1. Use ESPAsyncWebServer instead of ESP8266WebServer

Date: 2024-XX-XX

## Status

Accepted

## Context

The device needs to serve a web interface for configuration (Portal Mode) and expose an HTTP API for data access and control. The ESP8266 ecosystem offers two primary web server libraries:

1. **ESP8266WebServer** - Synchronous, blocking requests
2. **ESPAsyncWebServer** - Asynchronous, non-blocking, more efficient

The device also needs to:
- Handle MQTT operations simultaneously
- Take sensor readings every 30 seconds
- Maintain responsive LED control
- Support OTA updates on a separate port

## Decision

Use **ESPAsyncWebServer** for all HTTP serving needs.

## Consequences

**Positive:**
- Non-blocking request handling allows the device to continue sensor readings and MQTT operations while serving web requests
- More efficient memory usage
- Better performance under load
- Built-in support for WebSockets (future enhancement)

**Negative:**
- Slightly more complex API compared to synchronous server
- Requires additional dependency (AsyncTCP)

## Alternatives Considered

1. **ESP8266WebServer**: Simpler but blocking. Would require careful timing management to avoid delaying sensor readings during web requests.
2. **Custom lightweight server**: Too much development effort for limited benefit.
