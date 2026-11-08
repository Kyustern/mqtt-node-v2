# 5. Use EEPROM for Backup Credentials Storage

Date: 2024-XX-XX

## Status

Accepted

## Context

When the user triggers a clean reboot (`/reboot-clean`), the current credentials should be saved so they can be restored or referenced in Portal Mode. We need to choose a storage mechanism.

Options considered:
1. **EEPROM** - Emulated EEPROM in flash (4KB on ESP8266)
2. **LittleFS** - Flash filesystem, more space, file-based
3. **SPIFFS** - Older flash filesystem, being deprecated

The backup JSON structure:
```json
{
  "wifi": {"ssid": "...", "password": "..."},
  "mqtt": {"host": "...", "port": 1883, "username": "...", "password": "...", 
           "sensor_topic": "...", "led_topic": "..."}
}
```

Estimated size: ~200-300 bytes maximum.

## Decision

Use **EEPROM** to store backup credentials.

## Consequences

**Positive:**
- Simple API (built into ESP8266 Arduino core)
- No need for filesystem formatting or management
- Sufficient space (4KB for ~300 bytes of data)
- Faster read/write than filesystem
- No additional library dependencies

**Negative:**
- Limited to 4KB total (but we only need ~300 bytes)
- EEPROM has limited write cycles (~100,000), but backup is written rarely
- Data is not human-readable without special tools

## Alternatives Considered

1. **LittleFS**: More flexible, supports larger data, human-readable files. However, requires formatting the filesystem and adds complexity. Overkill for a single small JSON object.
2. **SPIFFS**: Legacy option, being replaced by LittleFS. Not recommended for new projects.
