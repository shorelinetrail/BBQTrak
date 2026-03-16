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
    if (millis() - lastWiFiCheck > 30000) {
        lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            WiFi.reconnect();
        }
    }
}
