# 2. Use float for Temperature and Humidity Values

Date: 2024-XX-XX

## Status

Accepted

## Context

The DHT22 sensor provides temperature measurements with 0.1°C resolution and humidity with 0.1% RH resolution. We need to store these values in the Rolling Buffer.

Two options were considered:
1. **float** (4 bytes each) - Full precision, direct representation
2. **int16_t** (2 bytes each) with scaling - Saves memory, requires multiplication/division

With 1000 readings in the buffer:
- float: 12 bytes/reading = 12,000 bytes total
- int16_t: 8 bytes/reading (4 for time + 2 for temp + 2 for humidity) = 8,000 bytes total

The ESP8266 typically has ~40-50KB of free heap, so both fit comfortably.

## Decision

Use **float** for temperature and humidity values.

## Consequences

**Positive:**
- Full precision maintained without scaling operations
- Simpler code (no need to multiply/divide by scaling factors)
- More flexible for future use cases
- Matches natural representation from DHT22 library

**Negative:**
- Uses 4 extra bytes per reading (4,000 bytes total for 1000 readings)
- Slightly less memory efficient

## Alternatives Considered

1. **int16_t with scaling**: Would save memory but add complexity and potential precision issues at the extremes of the DHT22's range (-40°C to +80°C, 0-100% RH).
