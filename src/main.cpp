#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "config.h"
#include "smoker_controller.h"
#include "web_server.h"
#include "index_html.h"

SmokerController smoker;
BBQWebServer webServer(80);

void connectWiFi() {
    Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Access BBQTrak at: http://%s.local\n", HOSTNAME);

        if (MDNS.begin(HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            Serial.println("mDNS responder started");
        }
    } else {
        // Fall back to AP mode so user can still access the interface
        Serial.println("\nWiFi connection failed. Starting AP mode...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("BBQTrak", "bbqtrak123");
        Serial.printf("AP started. Connect to 'BBQTrak' WiFi, then go to: http://%s\n",
                       WiFi.softAPIP().toString().c_str());
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println("  BBQTrak - Smoker Controller");
    Serial.println("=================================");
    Serial.println();

    smoker.begin();
    connectWiFi();
    webServer.begin(&smoker);

    Serial.println();
    Serial.println("System ready. Use web interface to control smoker.");
    Serial.println();
}

void loop() {
    smoker.update();

    // Reconnect WiFi if disconnected (STA mode only)
    static uint32_t lastWiFiCheck = 0;
    if (WiFi.getMode() == WIFI_STA && millis() - lastWiFiCheck > 30000) {
        lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            WiFi.reconnect();
        }
    }
}
