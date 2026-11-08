# 3. Use Captive Portal for Configuration

Date: 2024-XX-XX

## Status

Accepted

## Context

The device needs a user-friendly way to configure WiFi and MQTT settings when no valid configuration exists or when the user wants to change settings.

Options considered:
1. **Captive Portal** - Device creates its own WiFi network, intercepts DNS requests, redirects to config page
2. **Serial Console** - User connects via USB/serial to enter settings
3. **Hardcoded with compile-time changes** - Change settings and re-flash
4. **mDNS/Bonjour** - Device announces itself on local network, user visits device.local

## Decision

Use **Captive Portal** as the primary configuration method.

## Consequences

**Positive:**
- Works on any device with WiFi (phone, tablet, laptop)
- No need for USB cables or serial adapters
- User-friendly: any browser request redirects to config page
- Standard pattern for IoT devices (similar to Sonoff, Shelly, etc.)
- Provides fallback when WiFi credentials are wrong

**Negative:**
- Requires additional libraries (DNSServer)
- More complex to implement than serial console
- User must temporarily connect to device's WiFi network

## Alternatives Considered

1. **Serial Console**: Simple but requires physical access and USB connection. Not user-friendly for non-technical users.
2. **Hardcoded**: Inflexible. Requires re-flashing for any change. Not acceptable for deployment.
3. **mDNS**: Doesn't work when device isn't on the user's network (e.g., first setup). Still requires knowing the device's IP or hostname.
4. **Hybrid approach**: Could add serial console as fallback for advanced users, but captive portal is primary.
