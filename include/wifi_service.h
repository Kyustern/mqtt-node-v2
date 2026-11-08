#pragma once

#include <ESP8266WiFi.h>
#include "types.h"
#include "config.h"

class WiFiService {
public:
  // Constructor
  WiFiService();
  
  // Initialize WiFi
  void begin();
  
  // Connect to WiFi with stored credentials
  bool connect(const WiFiConfig& config);
  
  // Start Access Point mode for configuration
  void startAPMode();
  
  // Stop Access Point mode
  void stopAPMode();
  
  // Disconnect from WiFi
  void disconnect();
  
  // Check connection status
  bool isConnected() const;
  
  // Check if in AP mode
  bool isAPMode() const;
  
  // Get current IP address
  IPAddress getLocalIP() const;
  
  // Get WiFi RSSI
  int8_t getRSSI() const;
  
  // Get connection status as string
  String getStatusString() const;
  
  // Check if WiFi credentials are configured
  bool hasConfiguration() const;
  
  // Get current WiFi config (from stored or active connection)
  WiFiConfig getCurrentConfig() const;
  
private:
  bool _connected;
  bool _apMode;
  
  // Attempt connection with retry logic
  bool attemptConnection(const String& ssid, const String& password, 
                        uint8_t maxRetries = WIFI_CONNECTION_RETRIES);
};

// Global instance
extern WiFiService wifiService;
