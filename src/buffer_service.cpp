#include "buffer_service.h"
#include "config.h"
#include <ArduinoJson.h>
#include <string.h>

BufferService bufferService;

// Constructor
BufferService::BufferService() 
  : _head(0), _count(0) {
}

// Initialize buffer
void BufferService::begin() {
  clear();
  DEBUG_PRINTLN(F("Rolling buffer initialized"));
}

// Add a new reading to the buffer
void BufferService::addReading(const SensorReading& reading) {
  _buffer[_head] = reading;
  _head = indexOf(_head + 1);
  
  if (_count < BUFFER_CAPACITY) {
    _count++;
  }
  
  DEBUG_PRINT("Added reading to buffer. Count: ");
  DEBUG_PRINTLN(_count);
}

// Get all readings from the buffer
void BufferService::getReadings(SensorReading* output, size_t& count) const {
  if (output == nullptr || count == 0) {
    return;
  }
  
  size_t readingsToCopy = (count < _count) ? count : _count;
  
  // Start from the newest reading and go backwards
  BufferIndex start = indexOf(_head - 1);
  for (size_t i = 0; i < readingsToCopy; i++) {
    BufferIndex idx = indexOf(start - i);
    output[i] = _buffer[idx];
  }
  
  count = readingsToCopy;
}

// Get the latest reading
SensorReading BufferService::getLatest() const {
  if (_count == 0) {
    return SensorReading();
  }
  return _buffer[indexOf(_head - 1)];
}

// Get reading at specific index (0 = newest, BUFFER_CAPACITY-1 = oldest)
SensorReading BufferService::getAt(BufferIndex index) const {
  if (index >= _count || _count == 0) {
    return SensorReading();
  }
  BufferIndex actualIndex = indexOf(_head - 1 - index);
  return _buffer[actualIndex];
}

// Get current number of readings in buffer
size_t BufferService::count() const {
  return _count;
}

// Check if buffer is full
bool BufferService::isFull() const {
  return _count >= BUFFER_CAPACITY;
}

// Check if buffer is empty
bool BufferService::isEmpty() const {
  return _count == 0;
}

// Clear the buffer
void BufferService::clear() {
  _head = 0;
  _count = 0;
  // Use loop to properly initialize each SensorReading
  for (size_t i = 0; i < BUFFER_CAPACITY; i++) {
    _buffer[i] = SensorReading();
  }
  DEBUG_PRINTLN(F("Buffer cleared"));
}

// Get buffer capacity
size_t BufferService::capacity() const {
  return BUFFER_CAPACITY;
}

// Get readings as JSON array
String BufferService::toJson() const {
  // Calculate required buffer size
  // Each reading: {"time":12345678,"temperature":23.5,"humidity":45.2}
  // Approx 50 chars per reading, plus brackets and commas
  size_t bufferSize = _count * 60 + 10;
  
  DynamicJsonDocument doc(bufferSize);
  JsonArray array = doc.to<JsonArray>();
  
  // Add readings from newest to oldest
  for (BufferIndex i = 0; i < _count; i++) {
    BufferIndex idx = indexOf(_head - 1 - i);
    const SensorReading& reading = _buffer[idx];
    
    JsonObject obj = array.createNestedObject();
    obj["time"] = reading.time;
    obj["temperature"] = reading.temperature;
    obj["humidity"] = reading.humidity;
  }
  
  String result;
  serializeJson(doc, result);
  return result;
}

// Get index with wrap-around
BufferIndex BufferService::indexOf(BufferIndex offset) const {
  return offset % BUFFER_CAPACITY;
}
