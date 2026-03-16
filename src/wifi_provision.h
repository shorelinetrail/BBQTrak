#pragma once

#include <ESPAsyncWebServer.h>

// WiFi provisioning with captive portal.
// On first boot (or when stored creds fail), starts an AP with a config page.
// Credentials are stored in NVS (Preferences) and persist across reboots.
class WiFiProvisioner {
public:
    // Load saved credentials. Returns true if credentials exist.
    bool loadCredentials(String& ssid, String& password);

    // Save credentials to NVS
    void saveCredentials(const String& ssid, const String& password);

    // Clear saved credentials
    void clearCredentials();

    // Try connecting with saved credentials. Returns true on success.
    bool connectSaved(uint32_t timeoutMs = 15000);

    // Start AP mode with captive portal for configuration.
    // Serves a config page and handles credential submission.
    // Call update() in loop to handle DNS for captive portal.
    void startPortal(AsyncWebServer& server);

    // Must be called in loop() when portal is active
    void update();

    // Check if portal requested a reboot (after saving new creds)
    bool shouldReboot() const { return _shouldReboot; }

    bool isPortalActive() const { return _portalActive; }

private:
    bool _portalActive = false;
    bool _shouldReboot = false;
    void* _dns = nullptr;  // DNSServer pointer, void to avoid header dep
};
