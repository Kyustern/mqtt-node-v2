#pragma once

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "types.h"
#include "config.h"

class PortalService {
public:
  // Constructor
  PortalService();
  
  // Initialize portal service
  void begin();
  
  // Start configuration portal
  void startPortal();
  
  // Stop configuration portal
  void stopPortal();
  
  // Check if portal is running
  bool isRunning() const;
  
  // Process portal requests (call in main loop)
  void loop();
  
  // Save current configuration to EEPROM backup
  void saveBackupConfig(const DeviceConfig& config);
  
  // Load backup configuration from EEPROM
  bool loadBackupConfig(DeviceConfig& config) const;
  
  // Clear backup configuration from EEPROM
  void clearBackupConfig();
  
  // Check if backup configuration exists
  bool hasBackupConfig() const;
  
  // Save configuration to EEPROM (main config)
  void saveConfigToEEPROM(const DeviceConfig& config);
  
  // Load configuration from EEPROM (main config)
  bool loadConfigFromEEPROM(DeviceConfig& config) const;
  
private:
  ESP8266WebServer _server;
  DNSServer _dnsServer;
  bool _running;
  
  // Setup routes
  void setupRoutes();
  
  // Handle root page
  void handleRoot();
  
  // Handle form submission
  void handleConfigForm();
  
  // Handle 404
  void handleNotFound();
  
  // Generate HTML form
  String generateConfigForm(const DeviceConfig& currentConfig, 
                            const DeviceConfig& backupConfig);
};

// Global instance
extern PortalService portalService;
