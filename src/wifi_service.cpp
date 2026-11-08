#include "wifi_service.h"
#include "config.h"

WiFiService wifiService;

// Constructor
WiFiService::WiFiService() 
  : _connected(false), 
    _apMode(false) {
}

// Initialize WiFi
void WiFiService::begin() {
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  _connected = false;
  _apMode = false;
  DEBUG_PRINTLN(F("WiFi service initialized"));
}

// Connect to WiFi with stored credentials
bool WiFiService::connect(const WiFiConfig& config) {
  if (!config.isValid()) {
    DEBUG_PRINTLN(F("Invalid WiFi config"));
    return false;
  }
  
  _connected = attemptConnection(config.ssid, config.password);
  return _connected;
}

// Start Access Point mode for configuration
void WiFiService::startAPMode() {
  DEBUG_PRINTLN(F("Starting AP mode"));
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(DEFAULT_AP_IP, DEFAULT_AP_GATEWAY, DEFAULT_AP_SUBNET);
  WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
  
  _apMode = true;
  _connected = false;
  
  DEBUG_PRINT(F("AP started. SSID: "));
  DEBUG_PRINTLN(DEFAULT_AP_SSID);
  DEBUG_PRINT(F("AP IP: "));
  DEBUG_PRINTLN(WiFi.softAPIP());
}

// Stop Access Point mode
void WiFiService::stopAPMode() {
  if (_apMode) {
    WiFi.softAPdisconnect(true);
    _apMode = false;
    DEBUG_PRINTLN(F("AP mode stopped"));
  }
}

// Disconnect from WiFi
void WiFiService::disconnect() {
  WiFi.disconnect(true);
  _connected = false;
  DEBUG_PRINTLN(F("WiFi disconnected"));
}

// Check connection status
bool WiFiService::isConnected() const {
  if (_apMode) return false;
  return _connected && WiFi.status() == WL_CONNECTED;
}

// Check if in AP mode
bool WiFiService::isAPMode() const {
  return _apMode;
}

// Get current IP address
IPAddress WiFiService::getLocalIP() const {
  if (_apMode) {
    return WiFi.softAPIP();
  }
  return WiFi.localIP();
}

// Get WiFi RSSI
int8_t WiFiService::getRSSI() const {
  if (_apMode || !_connected) {
    return 0;
  }
  return WiFi.RSSI();
}

// Get connection status as string
String WiFiService::getStatusString() const {
  if (_apMode) {
    return F("AP Mode");
  }
  
  switch (WiFi.status()) {
    case WL_NO_SHIELD:        return F("No shield");
    case WL_IDLE_STATUS:      return F("Idle");
    case WL_NO_SSID_AVAIL:    return F("No SSID available");
    case WL_SCAN_COMPLETED:   return F("Scan completed");
    case WL_CONNECTED:        return F("Connected");
    case WL_CONNECT_FAILED:   return F("Connect failed");
    case WL_CONNECTION_LOST:  return F("Connection lost");
    case WL_DISCONNECTED:     return F("Disconnected");
    default:                  return F("Unknown");
  }
}

// Check if WiFi credentials are configured
bool WiFiService::hasConfiguration() const {
  return WiFi.SSID() != "";
}

// Get current WiFi config
WiFiConfig WiFiService::getCurrentConfig() const {
  WiFiConfig config;
  config.ssid = WiFi.SSID();
  config.password = WiFi.psk();
  return config;
}

// Attempt connection with retry logic
bool WiFiService::attemptConnection(const String& ssid, const String& password, 
                                    uint8_t maxRetries) {
  DEBUG_PRINTLN(F("\n[WIFI] ===== CONNECTION ATTEMPT ====="));
  DEBUG_PRINT(F("[WIFI] SSID: "));
  DEBUG_PRINTLN(ssid);
  DEBUG_PRINT(F("[WIFI] Password length: "));
  DEBUG_PRINT(password.length());
  DEBUG_PRINTLN(F(" chars"));
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    DEBUG_PRINT(F("[WIFI] Attempt "));
    DEBUG_PRINT(attempt + 1);
    DEBUG_PRINT(F("/"));
    DEBUG_PRINT(maxRetries);
    DEBUG_PRINT(F(" - Status: "));
    DEBUG_PRINTLN(getStatusString());
    
    delay(WIFI_RETRY_DELAY_MS);
    
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PRINTLN(F("[WIFI] SUCCESS: Connected!"));
      DEBUG_PRINT(F("[WIFI] IP: "));
      DEBUG_PRINTLN(WiFi.localIP());
      DEBUG_PRINT(F("[WIFI] RSSI: "));
      DEBUG_PRINT(WiFi.RSSI());
      DEBUG_PRINTLN(F(" dBm"));
      DEBUG_PRINT(F("[WIFI] Gateway: "));
      DEBUG_PRINTLN(WiFi.gatewayIP());
      DEBUG_PRINT(F("[WIFI] Subnet: "));
      DEBUG_PRINTLN(WiFi.subnetMask());
      DEBUG_PRINT(F("[WIFI] DNS: "));
      DEBUG_PRINTLN(WiFi.dnsIP());
      DEBUG_PRINTLN(F("[WIFI] ===== CONNECTION SUCCESSFUL =====\n"));
      return true;
    }
  }
  
  DEBUG_PRINTLN(F("[WIFI] ERROR: All attempts failed"));
  DEBUG_PRINT(F("[WIFI] Final status: "));
  DEBUG_PRINTLN(getStatusString());
  DEBUG_PRINTLN(F("[WIFI] ===== CONNECTION FAILED =====\n"));
  return false;
}
