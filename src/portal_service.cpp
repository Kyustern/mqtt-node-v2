#include "portal_service.h"
#include "config.h"
#include <ArduinoJson.h>
#include <EEPROM.h>

PortalService portalService;

// Constructor
PortalService::PortalService() 
  : _server(80),
    _dnsServer(),
    _running(false) {
}

// Initialize portal service
void PortalService::begin() {
  setupRoutes();
  _running = false;
  DEBUG_PRINTLN(F("Portal service initialized"));
}

// Start configuration portal
void PortalService::startPortal() {
  DEBUG_PRINTLN(F("Starting configuration portal"));
  
  // Start DNS server for captive portal
  _dnsServer.start(DNS_PORT, "*", DEFAULT_AP_IP);
  
  // Start web server
  _server.begin();
  _running = true;
  
  DEBUG_PRINT(F("Portal running on "));
  DEBUG_PRINT(DEFAULT_AP_IP);
  DEBUG_PRINTLN(F(":80"));
}

// Stop configuration portal
void PortalService::stopPortal() {
  if (_running) {
    _dnsServer.stop();
    _server.stop();
    _running = false;
    DEBUG_PRINTLN(F("Configuration portal stopped"));
  }
}

// Check if portal is running
bool PortalService::isRunning() const {
  return _running;
}

// Process portal requests (call in main loop)
void PortalService::loop() {
  if (_running) {
    _dnsServer.processNextRequest();
    _server.handleClient();
  }
}

// Save current configuration to EEPROM backup
void PortalService::saveBackupConfig(const DeviceConfig& config) {
  saveConfigToEEPROM(config);
  DEBUG_PRINTLN(F("Backup config saved to EEPROM"));
}

// Load backup configuration from EEPROM
bool PortalService::loadBackupConfig(DeviceConfig& config) const {
  return loadConfigFromEEPROM(config);
}

// Clear backup configuration from EEPROM
void PortalService::clearBackupConfig() {
  DeviceConfig empty;
  saveConfigToEEPROM(empty);
  DEBUG_PRINTLN(F("Backup config cleared"));
}

// Check if backup configuration exists
bool PortalService::hasBackupConfig() const {
  DeviceConfig config;
  return loadConfigFromEEPROM(config) && config.isValid();
}

// Setup routes
void PortalService::setupRoutes() {
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/config", HTTP_GET, [this]() { handleRoot(); }); // Alias for root
  _server.on("/save", HTTP_POST, [this]() { handleConfigForm(); });
  _server.onNotFound([this]() { handleNotFound(); });
  
  // Enable CORS
  _server.enableCORS(true);
}

// Handle root page
void PortalService::handleRoot() {
  DeviceConfig currentConfig;
  DeviceConfig backupConfig;
  
  // Try to load current config
  loadConfigFromEEPROM(currentConfig);
  
  // Try to load backup config
  loadBackupConfig(backupConfig);
  
  String html = generateConfigForm(currentConfig, backupConfig);
  _server.send(200, "text/html", html);
}

// Handle form submission
void PortalService::handleConfigForm() {
  DEBUG_PRINTLN(F("\n[PORTAL] ===== CONFIG FORM RECEIVED ====="));
  
  // Parse form data using built-in argument parsing (auto URL-decodes)
  DeviceConfig config;
  
  // Extract WiFi SSID
  if (_server.hasArg("ssid")) {
    config.wifi.ssid = _server.arg("ssid");
    DEBUG_PRINT(F("[PORTAL] SSID: "));
    DEBUG_PRINTLN(config.wifi.ssid);
  }
  
  // Extract WiFi Password
  if (_server.hasArg("password")) {
    config.wifi.password = _server.arg("password");
    DEBUG_PRINT(F("[PORTAL] WiFi Password: "));
    DEBUG_PRINT(config.wifi.password.length());
    DEBUG_PRINTLN(F(" chars"));
  }
  
  // Extract MQTT Host
  if (_server.hasArg("host")) {
    config.mqtt.host = _server.arg("host");
    DEBUG_PRINT(F("[PORTAL] MQTT Host: "));
    DEBUG_PRINTLN(config.mqtt.host);
  }
  
  // Extract MQTT Port
  if (_server.hasArg("port")) {
    String portStr = _server.arg("port");
    config.mqtt.port = portStr.toInt();
    if (config.mqtt.port == 0) config.mqtt.port = MQTT_DEFAULT_PORT;
    DEBUG_PRINT(F("[PORTAL] MQTT Port: "));
    DEBUG_PRINTLN(config.mqtt.port);
  }
  
  // Extract MQTT Username
  if (_server.hasArg("username")) {
    config.mqtt.username = _server.arg("username");
    DEBUG_PRINT(F("[PORTAL] MQTT Username: "));
    DEBUG_PRINTLN(config.mqtt.username);
  }
  
  // Extract MQTT Password
  if (_server.hasArg("mqtt_password")) {
    config.mqtt.password = _server.arg("mqtt_password");
    DEBUG_PRINT(F("[PORTAL] MQTT Password: "));
    DEBUG_PRINT(config.mqtt.password.length());
    DEBUG_PRINTLN(F(" chars"));
  }
  
  // Extract Sensor Topic
  if (_server.hasArg("sensor_topic")) {
    config.mqtt.sensor_topic = _server.arg("sensor_topic");
    DEBUG_PRINT(F("[PORTAL] Sensor Topic: "));
    DEBUG_PRINTLN(config.mqtt.sensor_topic);
  }
  
  // Extract LED Topic
  if (_server.hasArg("led_topic")) {
    config.mqtt.led_topic = _server.arg("led_topic");
    DEBUG_PRINT(F("[PORTAL] LED Topic: "));
    DEBUG_PRINTLN(config.mqtt.led_topic);
  }
  
  // Validate - WiFi SSID is required
  if (config.wifi.ssid.isEmpty()) {
    DEBUG_PRINTLN(F("[PORTAL] ERROR: WiFi SSID is required"));
    _server.send(400, "text/plain", "WiFi SSID is required");
    return;
  }
  
  // Log the configuration being saved
  DEBUG_PRINTLN(F("[PORTAL] Configuration validated. Saving..."));
  DEBUG_PRINT(F("[PORTAL] WiFi SSID: "));
  DEBUG_PRINTLN(config.wifi.ssid);
  DEBUG_PRINT(F("[PORTAL] WiFi Password: "));
  DEBUG_PRINT(config.wifi.password.length());
  DEBUG_PRINTLN(F(" chars"));
  DEBUG_PRINT(F("[PORTAL] MQTT Host: "));
  DEBUG_PRINTLN(config.mqtt.host);
  DEBUG_PRINT(F("[PORTAL] MQTT Port: "));
  DEBUG_PRINTLN(config.mqtt.port);
  DEBUG_PRINT(F("[PORTAL] MQTT Sensor Topic: "));
  DEBUG_PRINTLN(config.mqtt.sensor_topic);
  DEBUG_PRINT(F("[PORTAL] MQTT LED Topic: "));
  DEBUG_PRINTLN(config.mqtt.led_topic);
  
  // Check if MQTT will be used
  if (config.mqtt.isComplete()) {
    DEBUG_PRINTLN(F("[PORTAL] MQTT config complete - device will attempt MQTT connection"));
  } else {
    DEBUG_PRINTLN(F("[PORTAL] MQTT config incomplete - device will use Autonomous mode"));
  }
  
  // Save configuration
  saveConfigToEEPROM(config);
  
  // Save backup
  saveBackupConfig(config);
  
  DEBUG_PRINTLN(F("[PORTAL] Configuration saved to EEPROM"));
  DEBUG_PRINTLN(F("[PORTAL] ===== CONFIG SAVED =====\n"));
  
  // Respond with success
  _server.send(200, "text/plain", "OK");
  
  // Signal main loop to reboot
  // This will be handled by the main application
}

// Handle 404
void PortalService::handleNotFound() {
  // Redirect all requests to the portal
  String html = generateConfigForm(DeviceConfig(), DeviceConfig());
  _server.send(200, "text/html", html);
}

// Generate HTML form
String PortalService::generateConfigForm(const DeviceConfig& currentConfig, 
                                          const DeviceConfig& backupConfig) {
  String html = F("<html><head><title>ESP8266 Configuration</title>");
  html += F("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  html += F("<style>");
  html += F("body { font-family: Arial, sans-serif; max-width: 600px; margin: 20px; }");
  html += F("h1 { color: #333; }");
  html += F(".section { margin-bottom: 20px; border: 1px solid #ddd; padding: 10px; }");
  html += F(".section h2 { margin-top: 0; }");
  html += F("label { display: block; margin-top: 10px; }");
  html += F("input { width: 100%; padding: 8px; margin-top: 5px; box-sizing: border-box; }");
  html += F("button { background: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; margin-top: 10px; }");
  html += F("button:hover { background: #45a049; }");
  html += F(".backup { background: #fff8e1; padding: 10px; margin-top: 10px; border-radius: 5px; }");
  html += F("</style></head><body>");
  html += F("<h1>ESP8266 MQTT Node Configuration</h1>");
  
  // WiFi Section
  html += F("<div class='section'><h2>WiFi Configuration</h2>");
  html += F("<form method='POST' action='/save'>");
  html += F("<label for='ssid'>SSID:</label>");
  html += F("<input type='text' id='ssid' name='ssid' value='");
  html += currentConfig.wifi.ssid;
  html += F("' required><br>");
  
  html += F("<label for='password'>Password:</label>");
  html += F("<input type='text' id='password' name='password' value='");
  html += currentConfig.wifi.password;
  html += F("'><br>");
  
  html += F("</div>");
  
  // MQTT Section
  html += F("<div class='section'><h2>MQTT Configuration</h2>");
  html += F("<p style='color: #666; font-size: 0.9em;'>Note: Fill ALL MQTT fields to use MQTT mode. Leave any empty to use Autonomous mode (API only).</p>");
  html += F("<label for='host'>Broker Host:</label>");
  html += F("<input type='text' id='host' name='host' value='");
  html += currentConfig.mqtt.host;
  html += F("'><br>");
  
  html += F("<label for='port'>Port:</label>");
  html += F("<input type='number' id='port' name='port' value='");
  html += String(currentConfig.mqtt.port);
  html += F("' min='1' max='65535'><br>");
  
  html += F("<label for='username'>Username (optional):</label>");
  html += F("<input type='text' id='username' name='username' value='");
  html += currentConfig.mqtt.username;
  html += F("'><br>");
  
  html += F("<label for='mqtt_password'>Password (optional):</label>");
  html += F("<input type='text' id='mqtt_password' name='mqtt_password' value='");
  html += currentConfig.mqtt.password;
  html += F("'><br>");
  
  html += F("<label for='sensor_topic'>Sensor Topic:</label>");
  html += F("<input type='text' id='sensor_topic' name='sensor_topic' value='");
  html += currentConfig.mqtt.sensor_topic;
  html += F("'><br>");
  
  html += F("<label for='led_topic'>LED Control Topic:</label>");
  html += F("<input type='text' id='led_topic' name='led_topic' value='");
  html += currentConfig.mqtt.led_topic;
  html += F("'><br>");
  
  html += F("<button type='submit'>Save and Reboot</button>");
  html += F("</form></div>");
  
  // Backup Configuration Display
  if (backupConfig.isValid()) {
    html += F("<div class='section'><h2>Previous Configuration</h2>");
    html += F("<div class='backup'>");
    html += F("<p><strong>WiFi:</strong> ");
    html += backupConfig.wifi.ssid;
    html += F("</p>");
    html += F("<p><strong>MQTT:</strong> ");
    html += backupConfig.mqtt.host;
    html += F(":");
    html += String(backupConfig.mqtt.port);
    html += F("</p>");
    html += F("</div></div>");
  }
  
  html += F("<div class='section'><h2>Status</h2>");
  html += F("<p>Device will reboot with new configuration.</p>");
  html += F("</div>");
  
  html += F("</body></html>");
  
  return html;
}

// Save configuration to EEPROM
void PortalService::saveConfigToEEPROM(const DeviceConfig& config) {
  DynamicJsonDocument doc(512);
  
  JsonObject wifiObj = doc.createNestedObject("wifi");
  wifiObj["ssid"] = config.wifi.ssid;
  wifiObj["password"] = config.wifi.password;
  
  JsonObject mqttObj = doc.createNestedObject("mqtt");
  mqttObj["host"] = config.mqtt.host;
  mqttObj["port"] = config.mqtt.port;
  mqttObj["username"] = config.mqtt.username;
  mqttObj["password"] = config.mqtt.password;
  mqttObj["sensor_topic"] = config.mqtt.sensor_topic;
  mqttObj["led_topic"] = config.mqtt.led_topic;
  
  // Serialize to EEPROM
  EEPROM.begin(EEPROM_MAX_SIZE);
  
  String json;
  serializeJson(doc, json);
  
  // Write to EEPROM
  for (size_t i = 0; i < json.length(); i++) {
    EEPROM.write(i, json.charAt(i));
  }
  EEPROM.write(json.length(), '\0'); // Null terminator
  EEPROM.commit();
  
  DEBUG_PRINT(F("Config saved to EEPROM ("));
  DEBUG_PRINT(json.length());
  DEBUG_PRINTLN(F(" bytes)"));
}

// Load configuration from EEPROM
bool PortalService::loadConfigFromEEPROM(DeviceConfig& config) const {
  config.clear();
  
  EEPROM.begin(EEPROM_MAX_SIZE);
  
  // Read JSON from EEPROM
  String json;
  char c;
  size_t i = 0;
  
  while (i < EEPROM_MAX_SIZE) {
    c = EEPROM.read(i);
    if (c == '\0') break;
    if (c != 0xFF) { // Skip uninitialized EEPROM
      json += c;
    }
    i++;
  }
  
  if (json.isEmpty()) {
    DEBUG_PRINTLN(F("No config in EEPROM"));
    return false;
  }
  
  DEBUG_PRINT(F("Loaded config from EEPROM: "));
  DEBUG_PRINTLN(json);
  
  // Parse JSON
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT(F("Failed to parse config: "));
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // Extract WiFi config
  if (doc.containsKey("wifi")) {
    JsonObject wifiObj = doc["wifi"];
    config.wifi.ssid = wifiObj["ssid"] | "";
    config.wifi.password = wifiObj["password"] | "";
  }
  
  // Extract MQTT config
  if (doc.containsKey("mqtt")) {
    JsonObject mqttObj = doc["mqtt"];
    config.mqtt.host = mqttObj["host"] | "";
    config.mqtt.port = mqttObj["port"] | MQTT_DEFAULT_PORT;
    config.mqtt.username = mqttObj["username"] | "";
    config.mqtt.password = mqttObj["password"] | "";
    config.mqtt.sensor_topic = mqttObj["sensor_topic"] | MQTT_DEFAULT_SENSOR_TOPIC;
    config.mqtt.led_topic = mqttObj["led_topic"] | MQTT_DEFAULT_LED_TOPIC;
  }
  
  return config.isValid();
}
