#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "config.h"
#include "smoker_controller.h"
#include "web_server.h"
#include "wifi_provision.h"
#include "index_html.h"

SmokerController smoker;
BBQWebServer webServer(80);
WiFiProvisioner wifiProv;

bool portalMode = false;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println("  BBQTrak - Smoker Controller");
    Serial.println("=================================");
    Serial.println();

    smoker.begin();

    // Try connecting with saved WiFi credentials
    if (wifiProv.connectSaved()) {
        // Connected - start normal web server
        Serial.printf("Access BBQTrak at: http://%s.local\n", HOSTNAME);
        if (MDNS.begin(HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
        }
        webServer.begin(&smoker);
    } else {
        // No saved creds or connection failed - start provisioning portal
        portalMode = true;
        wifiProv.startPortal(webServer.getServer());
        Serial.println("Connect to WiFi 'BBQTrak-Setup' to configure");
    }

    Serial.println();
    Serial.println("System ready.");
    Serial.println();
}

void loop() {
    smoker.update();

    if (portalMode) {
        wifiProv.update();
        if (wifiProv.shouldReboot()) {
            Serial.println("Credentials saved. Rebooting...");
            delay(1000);
            ESP.restart();
        }
        return;
    }

    // Reconnect WiFi if disconnected (STA mode only)
    static uint32_t lastWiFiCheck = 0;
    static uint32_t wifiRetryDelay = 5000;   // Start at 5s, back off to 60s
    static uint8_t wifiFailCount = 0;
    uint32_t now = millis();

    if (now - lastWiFiCheck > wifiRetryDelay) {
        lastWiFiCheck = now;
        wl_status_t status = WiFi.status();
        if (status != WL_CONNECTED) {
            wifiFailCount++;
            Serial.printf("[WIFI] Disconnected (status=%d, attempt #%d, next retry %lums)\n",
                          status, wifiFailCount, wifiRetryDelay);
            WiFi.disconnect();
            delay(100);
            WiFi.reconnect();
            // Exponential backoff: 5s -> 10s -> 20s -> 40s -> 60s cap
            if (wifiRetryDelay < 60000) wifiRetryDelay *= 2;
            if (wifiRetryDelay > 60000) wifiRetryDelay = 60000;
        } else {
            if (wifiFailCount > 0) {
                Serial.printf("[WIFI] Reconnected after %d attempts (IP: %s, RSSI: %d dBm)\n",
                              wifiFailCount, WiFi.localIP().toString().c_str(), WiFi.RSSI());
            }
            wifiFailCount = 0;
            wifiRetryDelay = 5000;  // Reset backoff on success
        }
    }
}
