#pragma once

#include <FastLED.h>
#include "types.h"
#include "config.h"

class LEDService {
public:
  // Constructor
  LEDService();
  
  // Initialize LED strip
  void begin();
  
  // Update LEDs (call in main loop for animations)
  void loop();
  
  // Set all LEDs to a color
  void setAll(const CRGB& color);
  
  // Set all LEDs to a color with brightness
  void setAll(const CRGB& color, uint8_t brightness);
  
  // Set individual LED
  void setLED(uint8_t index, const CRGB& color);
  
  // Apply LED command
  void applyCommand(const LEDCommand& command);
  
  // Set LED strip enabled/disabled
  void setEnabled(bool enabled);
  
  // Get current enabled state
  bool isEnabled() const;
  
  // Set brightness (0-100)
  void setBrightness(uint8_t brightness);
  
  // Get current brightness
  uint8_t getBrightness() const;
  
  // Show boot animation
  void showBootAnimation();
  
  // Show status pattern for current mode
  void showStatusPattern(DeviceMode mode);
  
  // Show error pattern
  void showErrorPattern();
  
  // Show pulse animation
  void showPulse(CRGB color1, CRGB color2, uint8_t speed = LED_PULSE_SPEED);
  
  // Show chasing animation
  void showChase(CRGB color, uint8_t speed = LED_BOOT_ANIMATION_SPEED);
  
  // Clear all LEDs
  void clear();
  
  // Show current LED state
  void show();
  
  // Get current LED state as command
  LEDCommand getCurrentState() const;
  
private:
  CRGB _leds[LED_COUNT];
  LEDCommand _currentCommand;
  bool _fastLedInitialized = false;
  
  // Animation state
  unsigned long _lastAnimationTime;
  uint8_t _animationState;
  bool _animationActive;
  
  // Update animation
  void updateAnimation();
};

// Global instance
extern LEDService ledService;
