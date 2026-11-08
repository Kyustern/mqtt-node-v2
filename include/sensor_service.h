#pragma once

#include <DHT.h>
#include "types.h"
#include "config.h"

class SensorService {
public:
  // Constructor
  SensorService();
  
  // Initialize sensor
  bool begin();
  
  // Read sensor and return new reading
  SensorReading read();
  
  // Get last successful reading
  SensorReading getLastReading() const;
  
  // Check if sensor is initialized
  bool isInitialized() const;
  
  // Check if last reading was successful
  bool isLastReadingValid() const;
  
  // Get time of last reading
  unsigned long getLastReadingTime() const;
  
  // Get sensor status
  String getStatus() const;
  
private:
  DHT _dht;
  SensorReading _lastReading;
  bool _initialized;
  bool _lastReadSuccess;
  
  // Perform actual sensor reading
  bool performReading(float& temperature, float& humidity);
};

// Global instance
extern SensorService sensorService;
