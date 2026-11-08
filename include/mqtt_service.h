#pragma once

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "types.h"
#include "config.h"

class MQTTService {
public:
  // Constructor
  MQTTService();
  
  // Initialize MQTT client
  void begin();
  
  // Connect to MQTT broker
  bool connect(const MQTTConfig& config);
  
  // Disconnect from MQTT
  void disconnect();
  
  // Check if connected
  bool isConnected() const;
  
  // Publish sensor reading
  bool publishReading(const SensorReading& reading);
  
  // Publish a message to a topic
  bool publish(const String& topic, const String& payload, bool retain = false);
  
  // Subscribe to topics
  void subscribeToTopics(const MQTTConfig& config);
  
  // Process incoming MQTT messages (call in main loop)
  void loop();
  
  // Handle MQTT reconnection
  void reconnect();
  
  // Set current mode (affects reconnection behavior)
  void setMode(DeviceMode mode);
  
  // Set configuration
  void setConfig(const MQTTConfig& config);
  
  // Set LED command callback (to avoid direct dependency)
  void setLEDCommandCallback(void (*callback)(const String& payload));
  
private:
  WiFiClient _espClient;
  PubSubClient _mqttClient;
  
  MQTTConfig _config;
  DeviceMode _currentMode;
  unsigned long _lastReconnectAttempt;
  uint8_t _reconnectAttempts;
  
  // Callback for LED commands
  void (*_ledCommandCallback)(const String& payload);
  
  // MQTT callbacks
  static void _mqttCallback(char* topic, byte* payload, unsigned int length);
  void mqttCallback(char* topic, byte* payload, unsigned int length);
  
  // Process LED command from MQTT
  void processLEDCommand(const String& payload);
  
  // Attempt connection with retry
  bool attemptConnection();
};

// Global instance
extern MQTTService mqttService;
