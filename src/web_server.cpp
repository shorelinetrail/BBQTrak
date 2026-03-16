#include "web_server.h"
#include "smoker_controller.h"
#include "cook_profiles.h"
#include <ArduinoJson.h>
#include <math.h>

// Web UI served as a single embedded page
extern const char INDEX_HTML[] PROGMEM;

BBQWebServer::BBQWebServer(uint16_t port) : _server(port) {}

void BBQWebServer::begin(SmokerController* controller) {
    _ctrl = controller;
    setupRoutes();
    _server.begin();
    Serial.println("Web server started on port 80");
}

void BBQWebServer::setupRoutes() {
    // Serve the main page
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });

    // API endpoints
    _server.on("/api/status", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleGetStatus(r); });

    _server.on("/api/target", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleSetTarget(r); });

    _server.on("/api/pid", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleSetPID(r); });

    _server.on("/api/fan", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleSetFan(r); });

    _server.on("/api/history", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleGetHistory(r); });

    _server.on("/api/profiles", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleGetProfiles(r); });

    _server.on("/api/profile", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleSetProfile(r); });

    _server.on("/api/autotune", HTTP_GET,
        [this](AsyncWebServerRequest* r) { handleAutotune(r); });

    _server.on("/api/start", HTTP_GET,
        [this](AsyncWebServerRequest* r) {
            _ctrl->start();
            r->send(200, "application/json", "{\"ok\":true}");
        });

    _server.on("/api/stop", HTTP_GET,
        [this](AsyncWebServerRequest* r) {
            _ctrl->stop();
            r->send(200, "application/json", "{\"ok\":true}");
        });

    _server.on("/api/auto", HTTP_GET,
        [this](AsyncWebServerRequest* r) {
            _ctrl->setAutoMode();
            r->send(200, "application/json", "{\"ok\":true}");
        });
}

void BBQWebServer::handleGetStatus(AsyncWebServerRequest* request) {
    JsonDocument doc;

    doc["pit"] = isnan(_ctrl->getPitTempC()) ? 0 : _ctrl->getPitTempC();
    doc["meat"] = isnan(_ctrl->getMeatTempC()) ? 0 : _ctrl->getMeatTempC();
    doc["target"] = _ctrl->getTargetTempC();
    doc["eff_target"] = _ctrl->getEffectiveTargetC();
    doc["meat_target"] = _ctrl->getMeatTargetC();
    doc["fan"] = _ctrl->getFanSpeed();
    doc["pid_output"] = _ctrl->getPidOutput();
    doc["lid_open"] = _ctrl->isLidOpen();
    doc["pit_connected"] = _ctrl->isPitProbeConnected();
    doc["meat_connected"] = _ctrl->isMeatProbeConnected();
    doc["running"] = _ctrl->isRunning();
    doc["ramping_down"] = _ctrl->isRampingDown();
    doc["kp"] = _ctrl->getPID().getKp();
    doc["ki"] = _ctrl->getPID().getKi();
    doc["kd"] = _ctrl->getPID().getKd();
    doc["profile"] = _ctrl->getActiveProfile();
    doc["pid_zone"] = _ctrl->getActivePIDZoneName();
    doc["autotuning"] = _ctrl->isAutotuning();
    doc["autotune_done"] = _ctrl->isAutotuneComplete();

    if (_ctrl->isAutotuneComplete()) {
        auto r = _ctrl->getAutotuneResult();
        JsonObject at = doc["autotune_result"].to<JsonObject>();
        at["valid"] = r.valid;
        at["kp"] = r.kp;
        at["ki"] = r.ki;
        at["kd"] = r.kd;
        at["ku"] = r.ku;
        at["tu"] = r.tu;
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void BBQWebServer::handleSetTarget(AsyncWebServerRequest* request) {
    if (request->hasParam("pit")) {
        float temp = request->getParam("pit")->value().toFloat();
        _ctrl->setTargetTemp(temp);
    }
    if (request->hasParam("meat")) {
        float temp = request->getParam("meat")->value().toFloat();
        _ctrl->setMeatTarget(temp);
    }
    request->send(200, "application/json", "{\"ok\":true}");
}

void BBQWebServer::handleSetPID(AsyncWebServerRequest* request) {
    float kp = _ctrl->getPID().getKp();
    float ki = _ctrl->getPID().getKi();
    float kd = _ctrl->getPID().getKd();

    if (request->hasParam("kp")) kp = request->getParam("kp")->value().toFloat();
    if (request->hasParam("ki")) ki = request->getParam("ki")->value().toFloat();
    if (request->hasParam("kd")) kd = request->getParam("kd")->value().toFloat();

    _ctrl->setPIDTunings(kp, ki, kd);
    request->send(200, "application/json", "{\"ok\":true}");
}

void BBQWebServer::handleSetFan(AsyncWebServerRequest* request) {
    if (request->hasParam("speed")) {
        float speed = request->getParam("speed")->value().toFloat();
        _ctrl->setManualFanSpeed(speed);
    }
    request->send(200, "application/json", "{\"ok\":true}");
}

void BBQWebServer::handleGetHistory(AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray pitArr = doc["pit"].to<JsonArray>();
    JsonArray meatArr = doc["meat"].to<JsonArray>();
    JsonArray timeArr = doc["time"].to<JsonArray>();

    size_t count = _ctrl->getHistoryCount();
    size_t idx = _ctrl->getHistoryIndex();
    const TempReading* history = _ctrl->getHistory();

    // Output oldest-to-newest
    size_t start = (count < HISTORY_SIZE) ? 0 : idx;
    for (size_t i = 0; i < count; i++) {
        size_t pos = (start + i) % HISTORY_SIZE;
        pitArr.add(isnan(history[pos].pitC) ? 0 : history[pos].pitC);
        meatArr.add(isnan(history[pos].meatC) ? 0 : history[pos].meatC);
        timeArr.add(history[pos].timestamp / 1000);  // seconds
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void BBQWebServer::handleGetProfiles(AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (size_t i = 0; i < COOK_PROFILE_COUNT; i++) {
        JsonObject p = arr.add<JsonObject>();
        p["id"] = i;
        p["name"] = COOK_PROFILES[i].name;
        p["pit"] = COOK_PROFILES[i].pitTargetC;
        p["meat"] = COOK_PROFILES[i].meatTargetC;
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void BBQWebServer::handleSetProfile(AsyncWebServerRequest* request) {
    if (request->hasParam("id")) {
        int id = request->getParam("id")->value().toInt();
        _ctrl->setProfile(id);
    }
    request->send(200, "application/json", "{\"ok\":true}");
}

void BBQWebServer::handleAutotune(AsyncWebServerRequest* request) {
    if (request->hasParam("action")) {
        String action = request->getParam("action")->value();
        if (action == "start") {
            _ctrl->startAutotune();
        } else if (action == "cancel") {
            _ctrl->cancelAutotune();
        } else if (action == "apply") {
            _ctrl->applyAutotuneResult();
        }
    }
    request->send(200, "application/json", "{\"ok\":true}");
}
