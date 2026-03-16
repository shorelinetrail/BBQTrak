#pragma once

#include <ESPAsyncWebServer.h>

// Forward declarations
class SmokerController;

class BBQWebServer {
public:
    explicit BBQWebServer(uint16_t port = 80);

    void begin(SmokerController* controller);

private:
    AsyncWebServer _server;
    SmokerController* _ctrl = nullptr;

    void setupRoutes();
    void handleGetStatus(AsyncWebServerRequest* request);
    void handleSetTarget(AsyncWebServerRequest* request);
    void handleSetPID(AsyncWebServerRequest* request);
    void handleSetFan(AsyncWebServerRequest* request);
    void handleGetHistory(AsyncWebServerRequest* request);
};
