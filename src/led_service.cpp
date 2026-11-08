#include "led_service.h"
#include "config.h"

LEDService ledService;

// Constructor
LEDService::LEDService() 
  : _currentCommand(), 
    _lastAnimationTime(0), 
    _animationState(0), 
    _animationActive(false) {
}

// Initialize LED strip
void LEDService::begin() {
  // For SK6812 RGBW LEDs - TEMPORARILY DISABLED
  // FastLED.addLeds<SK6812, LED_PIN, RGB>(_leds, LED_COUNT);
  // if successful:
  _fastLedInitialized = false; // TEMP DISABLED
  // FastLED.setBrightness(100);
  // FastLED.clear();
  // FastLED.show();
  
  _currentCommand.brightness = 100;
  _currentCommand.enabled = false;
  
  DEBUG_PRINTLN(F("LED service initialized (FastLED disabled)"));
}

// Update LEDs (call in main loop for animations)
void LEDService::loop() {
  if (_animationActive) {
    updateAnimation();
  }
}

// Set all LEDs to a color
void LEDService::setAll(const CRGB& color) {
  // TEMP DISABLED
  // for (int i = 0; i < LED_COUNT; i++) {
  //   _leds[i] = color;
  // }
  // _animationActive = false;
  // FastLED.show();
}

// Set all LEDs to a color with brightness
void LEDService::setAll(const CRGB& color, uint8_t brightness) {
  // TEMP DISABLED
  // FastLED.setBrightness(brightness);
  // setAll(color);
}

// Set individual LED
void LEDService::setLED(uint8_t index, const CRGB& color) {
  // TEMP DISABLED
  // if (index < LED_COUNT) {
  //   _leds[index] = color;
  //   FastLED.show();
  // }
}

// Apply LED command
void LEDService::applyCommand(const LEDCommand& command) {
  if (!command.isValid()) {
    DEBUG_PRINTLN(F("Invalid LED command received"));
    return;
  }
  
  _currentCommand = command;
  _animationActive = false;
  
  // TEMP DISABLED
  // if (command.enabled) {
  //   CRGB rgbColor(command.r, command.g, command.b);
  //   setAll(rgbColor);
  //   FastLED.setBrightness(command.brightness * 2.55);
  //   FastLED.show();
  // } else {
  //   clear();
  // }
  
  DEBUG_PRINT("LED command applied: r=");
  DEBUG_PRINT(command.r);
  DEBUG_PRINT(", g=");
  DEBUG_PRINT(command.g);
  DEBUG_PRINT(", b=");
  DEBUG_PRINT(command.b);
  DEBUG_PRINT(", w=");
  DEBUG_PRINT(command.w);
  DEBUG_PRINT(", brightness=");
  DEBUG_PRINT(command.brightness);
  DEBUG_PRINT(", enabled=");
  DEBUG_PRINTLN(command.enabled);
}

// Set LED strip enabled/disabled
void LEDService::setEnabled(bool enabled) {
  _currentCommand.enabled = enabled;
  if (!enabled) {
    clear();
  } else {
    applyCommand(_currentCommand);
  }
}

// Get current enabled state
bool LEDService::isEnabled() const {
  return _currentCommand.enabled;
}

// Set brightness (0-100)
void LEDService::setBrightness(uint8_t brightness) {
  _currentCommand.brightness = brightness;
  // TEMP DISABLED
  // FastLED.setBrightness(brightness * 2.55);
  // FastLED.show();
}

// Get current brightness
uint8_t LEDService::getBrightness() const {
  return _currentCommand.brightness;
}

// Show boot animation
void LEDService::showBootAnimation() {
  // TEMP DISABLED
  // _animationActive = true;
  // _animationState = 0;
  // _lastAnimationTime = millis();
  // for (int i = 0; i < LED_COUNT; i++) {
  //   _leds[i] = CRGB::Black;
  // }
  // FastLED.show();
}

// Show status pattern for current mode
void LEDService::showStatusPattern(DeviceMode mode) {
  // TEMP DISABLED
  // _animationActive = false;
  // switch (mode) {
  //   case DeviceMode::PORTAL:
  //     setAll(LED_PORTAL_COLOR);
  //     _animationActive = true;
  //     _animationState = 0;
  //     break;
  //   case DeviceMode::MQTT:
  //     setAll(LED_MQTT_COLOR);
  //     break;
  //   case DeviceMode::AUTONOMOUS:
  //     setAll(LED_AUTONOMOUS_COLOR);
  //     break;
  //   default:
  //     setAll(LED_ERROR_COLOR);
  //     break;
  // }
}

// Show error pattern
void LEDService::showErrorPattern() {
  // TEMP DISABLED
  // _animationActive = false;
  // setAll(LED_ERROR_COLOR);
}

// Show pulse animation
void LEDService::showPulse(CRGB color1, CRGB color2, uint8_t speed) {
  // TEMP DISABLED
  // _animationActive = true;
  // _animationState = 0;
  // _lastAnimationTime = millis();
  // for (int i = 0; i < LED_COUNT; i++) {
  //   _leds[i] = color1;
  // }
  // FastLED.show();
}

// Show chasing animation
void LEDService::showChase(CRGB color, uint8_t speed) {
  // TEMP DISABLED
  // _animationActive = true;
  // _animationState = 0;
  // _lastAnimationTime = millis();
  // for (int i = 0; i < LED_COUNT; i++) {
  //   _leds[i] = CRGB::Black;
  // }
  // FastLED.show();
}

// Clear all LEDs
void LEDService::clear() {
  // TEMP DISABLED
  // FastLED.clear();
  // FastLED.show();
  // _animationActive = false;
}

// Show current LED state
void LEDService::show() {
  // TEMP DISABLED
  // FastLED.show();
}

// Get current LED state as command
LEDCommand LEDService::getCurrentState() const {
  return _currentCommand;
}

// Update animation
void LEDService::updateAnimation() {
  // TEMP DISABLED
  // unsigned long now = millis();
  // if (_currentCommand.enabled == false) {
  //   uint8_t brightness = 128 + 127 * sin(_animationState * 0.1);
  //   FastLED.setBrightness(brightness);
  //   for (int i = 0; i < LED_COUNT; i++) {
  //     _leds[i] = LED_PORTAL_COLOR;
  //   }
  //   FastLED.show();
  //   _animationState = (now / 50) % 255;
  //   _lastAnimationTime = now;
  // }
}
