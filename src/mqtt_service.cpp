#include "mqtt_service.h"
#include "config.h"
#include <ArduinoJson.h>

MQTTService mqttService;

// Constructor
MQTTService::MQTTService() 
  : _espClient(),
    _mqttClient(_espClient),
    _config(),
    _currentMode(DeviceMode::UNKNOWN),
    _lastReconnectAttempt(0),
    _reconnectAttempts(0),
    _ledCommandCallback(nullptr) {
}

// Initialize MQTT client
void MQTTService::begin() {
  _mqttClient.setServer(_config.host.c_str(), _config.port);
  _mqttClient.setCallback(_mqttCallback);
  DEBUG_PRINTLN(F("MQTT service initialized"));
}

// Connect to MQTT broker
bool MQTTService::connect(const MQTTConfig& config) {
  _config = config;
  _mqttClient.setServer(_config.host.c_str(), _config.port);
  
  DEBUG_PRINTLN(F("\n[MQTT] ===== CONNECTION ATTEMPT ====="));
  DEBUG_PRINT(F("[MQTT] Broker: "));
  DEBUG_PRINT(_config.host);
  DEBUG_PRINT(F(":"));
  DEBUG_PRINTLN(_config.port);
  DEBUG_PRINT(F("[MQTT] Sensor topic: "));
  DEBUG_PRINTLN(_config.sensor_topic);
  DEBUG_PRINT(F("[MQTT] LED topic: "));
  DEBUG_PRINTLN(_config.led_topic);
  DEBUG_PRINT(F("[MQTT] Username: "));
  DEBUG_PRINTLN(_config.username.isEmpty() ? "(none)" : _config.username);
  DEBUG_PRINT(F("[MQTT] Using auth: "));
  DEBUG_PRINTLN(_config.username.length() > 0 ? "Yes" : "No");
  
  String clientId = F("ESP8266-");
  clientId += ESP.getChipId();
  DEBUG_PRINT(F("[MQTT] Client ID: "));
  DEBUG_PRINTLN(clientId);
  
  if (_config.username.length() > 0) {
    DEBUG_PRINTLN(F("[MQTT] Connecting with username/password..."));
    if (_mqttClient.connect(clientId.c_str(), _config.username.c_str(), _config.password.c_str())) {
      DEBUG_PRINTLN(F("[MQTT] SUCCESS: Connected to broker!"));
      subscribeToTopics(_config);
      DEBUG_PRINTLN(F("[MQTT] ===== CONNECTION SUCCESSFUL =====\n"));
      return true;
    } else {
      DEBUG_PRINT(F("[MQTT] ERROR: Connection failed. State: "));
      DEBUG_PRINTLN(_mqttClient.state());
    }
  } else {
    DEBUG_PRINTLN(F("[MQTT] Connecting without auth..."));
    if (_mqttClient.connect(clientId.c_str())) {
      DEBUG_PRINTLN(F("[MQTT] SUCCESS: Connected to broker!"));
      subscribeToTopics(_config);
      DEBUG_PRINTLN(F("[MQTT] ===== CONNECTION SUCCESSFUL =====\n"));
      return true;
    } else {
      DEBUG_PRINT(F("[MQTT] ERROR: Connection failed. State: "));
      DEBUG_PRINTLN(_mqttClient.state());
    }
  }
  
  DEBUG_PRINTLN(F("[MQTT] ===== CONNECTION FAILED =====\n"));
  return false;
}

// Disconnect from MQTT
void MQTTService::disconnect() {
  _mqttClient.disconnect();
  _reconnectAttempts = 0;
  _lastReconnectAttempt = 0;
  DEBUG_PRINTLN(F("Disconnected from MQTT"));
}

// Check if connected
bool MQTTService::isConnected() const {
  // const_cast to call non-const method
  return const_cast<MQTTService*>(this)->_mqttClient.connected();
}

// Publish sensor reading
bool MQTTService::publishReading(const SensorReading& reading) {
  if (!isConnected() || _config.sensor_topic.isEmpty()) {
    return false;
  }
  
  DynamicJsonDocument doc(256);
  doc["time"] = reading.time;
  doc["temperature"] = reading.temperature;
  doc["humidity"] = reading.humidity;
  
  String payload;
  serializeJson(doc, payload);
  
  return publish(_config.sensor_topic, payload, true);
}

// Publish a message to a topic
bool MQTTService::publish(const String& topic, const String& payload, bool retain) {
  if (!isConnected() || topic.isEmpty()) {
    return false;
  }
  
  bool success = _mqttClient.publish(topic.c_str(), payload.c_str(), retain);
  if (success) {
    DEBUG_PRINT(F("Published to "));
    DEBUG_PRINT(topic);
    DEBUG_PRINT(F(": "));
    DEBUG_PRINTLN(payload);
  } else {
    DEBUG_PRINT(F("Failed to publish to "));
    DEBUG_PRINTLN(topic);
  }
  return success;
}

// Subscribe to topics
void MQTTService::subscribeToTopics(const MQTTConfig& config) {
  if (!isConnected()) {
    return;
  }
  
  if (!config.led_topic.isEmpty()) {
    _mqttClient.subscribe(config.led_topic.c_str());
    DEBUG_PRINT(F("Subscribed to: "));
    DEBUG_PRINTLN(config.led_topic);
  }
}

// Process incoming MQTT messages (call in main loop)
void MQTTService::loop() {
  _mqttClient.loop();
  
  // Handle reconnection if needed
  if (_currentMode == DeviceMode::MQTT && !isConnected()) {
    reconnect();
  }
}

// Handle MQTT reconnection
void MQTTService::reconnect() {
  unsigned long now = millis();
  
  // Only attempt reconnection periodically in autonomous mode
  if (_currentMode == DeviceMode::AUTONOMOUS) {
    if (now - _lastReconnectAttempt < MQTT_AUTONOMOUS_RETRY_MS) {
      DEBUG_PRINTLN(F("[MQTT] Autonomous mode: waiting for retry window"));
      return;
    }
    _lastReconnectAttempt = now;
    DEBUG_PRINT(F("[MQTT] Autonomous mode: attempt "));
    DEBUG_PRINT(_reconnectAttempts);
    DEBUG_PRINTLN(F(" to reconnect"));
  }
  
  // For MQTT mode, use exponential backoff
  if (_currentMode == DeviceMode::MQTT) {
    unsigned long backoff = MQTT_RECONNECT_DELAY_MS * (1 << _reconnectAttempts);
    if (now - _lastReconnectAttempt < backoff) {
      DEBUG_PRINT(F("[MQTT] MQTT mode: backoff wait (ms): "));
      DEBUG_PRINTLN(backoff - (now - _lastReconnectAttempt));
      return;
    }
    _lastReconnectAttempt = now;
    
    if (_reconnectAttempts < MQTT_MAX_RECONNECT_ATTEMPTS) {
      _reconnectAttempts++;
      DEBUG_PRINT(F("[MQTT] MQTT mode: attempt "));
      DEBUG_PRINT(_reconnectAttempts);
      DEBUG_PRINT(F("/"));
      DEBUG_PRINT(MQTT_MAX_RECONNECT_ATTEMPTS);
      DEBUG_PRINTLN(F(" to reconnect"));
    } else {
      // Too many retries, give up and go to autonomous
      DEBUG_PRINTLN(F("[MQTT] ERROR: Max reconnection attempts reached. Will fallback to Autonomous mode"));
      _reconnectAttempts = 0;
      return;
    }
  }
  
  String clientId = F("ESP8266-");
  clientId += ESP.getChipId();
  DEBUG_PRINT(F("[MQTT] Connecting with client ID: "));
  DEBUG_PRINTLN(clientId);
  
  bool connected = false;
  if (_config.username.length() > 0) {
    DEBUG_PRINTLN(F("[MQTT] Using username/password auth"));
    connected = _mqttClient.connect(clientId.c_str(), _config.username.c_str(), _config.password.c_str());
  } else {
    DEBUG_PRINTLN(F("[MQTT] No auth"));
    connected = _mqttClient.connect(clientId.c_str());
  }
  
  if (connected) {
    DEBUG_PRINTLN(F("[MQTT] SUCCESS: Reconnected to broker!"));
    _reconnectAttempts = 0;
    subscribeToTopics(_config);
    _currentMode = DeviceMode::MQTT;
    DEBUG_PRINTLN(F("[MQTT] Mode set to MQTT"));
  } else {
    DEBUG_PRINT(F("[MQTT] ERROR: Reconnection failed. State: "));
    DEBUG_PRINTLN(_mqttClient.state());
  }
}

// Set current mode
void MQTTService::setMode(DeviceMode mode) {
  _currentMode = mode;
  
  if (mode == DeviceMode::MQTT) {
    _reconnectAttempts = 0;
  }
}

// Set configuration
void MQTTService::setConfig(const MQTTConfig& config) {
  _config = config;
  _mqttClient.setServer(_config.host.c_str(), _config.port);
}

// Set LED command callback
void MQTTService::setLEDCommandCallback(void (*callback)(const String& payload)) {
  _ledCommandCallback = callback;
}

// Static MQTT callback
void MQTTService::_mqttCallback(char* topic, byte* payload, unsigned int length) {
  mqttService.mqttCallback(topic, payload, length);
}

// MQTT callback
void MQTTService::mqttCallback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT(F("MQTT message received on topic: "));
  DEBUG_PRINTLN(topic);
  
  String topicStr = String(topic);
  String payloadStr;
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }
  
  DEBUG_PRINT(F("Payload: "));
  DEBUG_PRINTLN(payloadStr);
  
  // Handle LED control topic
  if (topicStr == _config.led_topic && _ledCommandCallback != nullptr) {
    _ledCommandCallback(payloadStr);
  }
}
