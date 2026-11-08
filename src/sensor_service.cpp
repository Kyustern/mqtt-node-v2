#include "sensor_service.h"
#include "config.h"

SensorService sensorService;

// Constructor
SensorService::SensorService() 
  : _dht(DHT_PIN, DHT_TYPE), 
    _lastReading(), 
    _initialized(false), 
    _lastReadSuccess(false) {
}

// Initialize sensor
bool SensorService::begin() {
  _dht.begin();
  _initialized = true;
  DEBUG_PRINTLN(F("DHT22 sensor initialized"));
  return _initialized;
}

// Read sensor and return new reading
SensorReading SensorService::read() {
  float temperature = 0.0;
  float humidity = 0.0;
  
  _lastReadSuccess = performReading(temperature, humidity);
  
  if (_lastReadSuccess) {
    unsigned long now = millis();
    _lastReading = SensorReading(now, temperature, humidity);
    DEBUG_PRINT("Sensor reading: temp=");
    DEBUG_PRINT(temperature);
    DEBUG_PRINT("C, humidity=");
    DEBUG_PRINT(humidity);
    DEBUG_PRINT("%, time=");
    DEBUG_PRINTLN(now);
  } else {
    DEBUG_PRINTLN(F("Failed to read from DHT22 sensor"));
  }
  
  return _lastReading;
}

// Get last successful reading
SensorReading SensorService::getLastReading() const {
  return _lastReading;
}

// Check if sensor is initialized
bool SensorService::isInitialized() const {
  return _initialized;
}

// Check if last reading was successful
bool SensorService::isLastReadingValid() const {
  return _lastReadSuccess && _lastReading.isValid();
}

// Get time of last reading
unsigned long SensorService::getLastReadingTime() const {
  return _lastReading.time;
}

// Get sensor status
String SensorService::getStatus() const {
  if (!_initialized) {
    return F("Not initialized");
  }
  if (!_lastReadSuccess) {
    return F("Read error");
  }
  return F("OK");
}

// Perform actual sensor reading
bool SensorService::performReading(float& temperature, float& humidity) {
  if (!_initialized) {
    return false;
  }
  
  // DHT22 can take up to 2 seconds to read
  float temp = _dht.readTemperature();
  float hum = _dht.readHumidity();
  
  // Check for NaN values
  if (isnan(temp) || isnan(hum)) {
    DEBUG_PRINTLN(F("DHT22 returned NaN"));
    return false;
  }
  
  // Check for reasonable ranges
  if (temp < -50 || temp > 100 || hum < 0 || hum > 100) {
    DEBUG_PRINT("DHT22 values out of range: temp=");
    DEBUG_PRINT(temp);
    DEBUG_PRINT(", humidity=");
    DEBUG_PRINTLN(hum);
    return false;
  }
  
  temperature = temp;
  humidity = hum;
  return true;
}
