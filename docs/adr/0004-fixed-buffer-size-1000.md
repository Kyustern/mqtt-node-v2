# 4. Fixed Rolling Buffer Size of 1000 Readings

Date: 2024-XX-XX

## Status

Accepted

## Context

In Autonomous Mode, the device stores sensor readings in a rolling buffer. We need to determine the buffer size.

Options considered:
1. **Fixed size** - Pre-allocate a specific number of entries
2. **Dynamic size** - Calculate maximum size at runtime based on available heap
3. **Time-based** - Store readings for a fixed duration (e.g., 24 hours)

Calculation for fixed size:
- Each reading: 12 bytes (4 for time + 4 for temp + 4 for humidity)
- 1000 readings: 12,000 bytes (12 KB)
- ESP8266 free heap: ~40-50 KB
- Remaining for other operations: ~28-38 KB (sufficient)

At 30-second intervals:
- 1000 readings = 1000 × 30s = 30,000s = 8.33 hours of history

## Decision

Use a **fixed size of 1000 readings** for the rolling buffer.

## Consequences

**Positive:**
- Predictable memory usage
- Simple implementation (fixed-size array)
- Guaranteed to fit on all ESP8266 devices
- 8.3 hours of history provides good visibility into recent conditions
- Easy to understand and reason about

**Negative:**
- Doesn't utilize all available memory on devices with more free heap
- Fixed time window (8.3 hours) may be too short or too long depending on use case

## Alternatives Considered

1. **Dynamic size**: Would maximize memory usage but adds complexity. Runtime calculation requires checking heap, and available heap can vary. Also, buffer would need to be reallocated if heap changes, which is complex on embedded systems.
2. **Time-based (24 hours)**: Would require ~2,880 readings (24h × 2 readings/min), using ~34.5 KB. Leaves less room for other operations and may not fit on all devices.
3. **Configurable size**: Adds complexity to configuration UI for minimal benefit.
