# 6. Use FastLED Library for SK6812 RGBW LED Control

Date: 2024-XX-XX

## Status

Accepted

## Context

The SK6812 is an RGBW addressable LED. We need a library to control it.

Options considered:
1. **FastLED** - Popular, feature-rich, supports RGBW
2. **Adafruit NeoPixel** - Simpler, widely used, but RGBW support is limited
3. **Custom bit-banging** - Full control but complex and error-prone

## Decision

Use **FastLED** library for SK6812 LED control.

## Consequences

**Positive:**
- Full support for SK6812 RGBW LEDs (including the white channel)
- Rich feature set: color correction, brightness control, animations
- Well-maintained, widely used in the community
- Efficient timing, works well with other operations
- Supports multiple LED types and configurations

**Negative:**
- Larger library size compared to NeoPixel
- Slightly more complex API
- Additional dependency

## Alternatives Considered

1. **Adafruit NeoPixel**: Simpler API but has limited RGBW support. The white channel on SK6812 is a separate LED, and NeoPixel treats it as part of the RGB channels, resulting in color inaccuracies.
2. **Custom bit-banging**: Provides full control but requires precise timing implementation. High development effort, prone to bugs, and difficult to maintain.
