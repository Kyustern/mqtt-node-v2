#pragma once

#include "types.h"
#include "config.h"

class BufferService {
public:
  // Constructor
  BufferService();
  
  // Initialize buffer
  void begin();
  
  // Add a new reading to the buffer
  void addReading(const SensorReading& reading);
  
  // Get all readings from the buffer
  void getReadings(SensorReading* output, size_t& count) const;
  
  // Get the latest reading
  SensorReading getLatest() const;
  
  // Get reading at specific index (0 = newest, BUFFER_CAPACITY-1 = oldest)
  SensorReading getAt(BufferIndex index) const;
  
  // Get current number of readings in buffer
  size_t count() const;
  
  // Check if buffer is full
  bool isFull() const;
  
  // Check if buffer is empty
  bool isEmpty() const;
  
  // Clear the buffer
  void clear();
  
  // Get buffer capacity
  size_t capacity() const;
  
  // Get readings as JSON array
  String toJson() const;
  
private:
  SensorReading _buffer[BUFFER_CAPACITY];
  BufferIndex _head;  // Points to next write position
  BufferIndex _count;
  
  // Get index with wrap-around
  BufferIndex indexOf(BufferIndex offset) const;
};

// Global instance
extern BufferService bufferService;
