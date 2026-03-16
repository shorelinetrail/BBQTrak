#include "smoker_controller.h"
#include <Arduino.h>
#include <SPI.h>
#include <math.h>

SmokerController::SmokerController()
    : _pitProbe(PIN_CS_PIT),
      _meatProbe(PIN_CS_MEAT),
      _pid(PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT,
           PID_OUTPUT_MIN, PID_OUTPUT_MAX) {}

void SmokerController::begin() {
    SPI.begin(PIN_SPI_CLK, PIN_SPI_MISO, -1, -1);  // CLK, MISO, no MOSI, no SS

    _pitProbe.begin();
    _meatProbe.begin();
    _fan.begin();

    Serial.printf("Pit probe: %s\n",
                  _pitProbe.isConnected() ? "connected" : "not found");
    Serial.printf("Meat probe: %s\n",
                  _meatProbe.isConnected() ? "connected" : "not found");
}

void SmokerController::update() {
    uint32_t now = millis();

    // Read sensors
    if (now - _lastTempRead >= TEMP_READ_INTERVAL_MS) {
        _lastTempRead = now;
        readSensors();
        recordHistory();
    }

    // Run control loop
    if (now - _lastControl >= CONTROL_INTERVAL_MS) {
        _lastControl = now;
        detectLidOpen();
        runControl();
    }

    // Update fan (handles kick-start timing)
    _fan.update();
}

void SmokerController::readSensors() {
    float reading = _pitProbe.readTempC();
    if (!isnan(reading)) {
        _prevPitTemp = _pitTemp;
        _pitTemp = reading;
    }

    reading = _meatProbe.readTempC();
    if (!isnan(reading)) {
        _meatTemp = reading;
    }
}

void SmokerController::runControl() {
    if (!_running || isnan(_pitTemp)) {
        if (!_manualFan) {
            _fan.setSpeed(0);
            _pidOutput = 0;
        }
        return;
    }

    if (_manualFan) {
        _fan.setSpeed(_manualFanSpeed);
        _pidOutput = _manualFanSpeed;
        return;
    }

    // Pause control during lid-open event
    if (_lidOpen) {
        _fan.setSpeed(0);
        _pidOutput = 0;
        return;
    }

    float dt = CONTROL_INTERVAL_MS / 1000.0f;
    _pidOutput = _pid.compute(_targetTemp, _pitTemp, dt);
    _fan.setSpeed(_pidOutput);
}

void SmokerController::detectLidOpen() {
    uint32_t now = millis();

    // Check if lid-open pause is still active
    if (_lidOpen && now < _lidOpenUntil) {
        return;
    } else if (_lidOpen && now >= _lidOpenUntil) {
        _lidOpen = false;
        _pid.reset();  // Reset PID to prevent integral windup overshoot
        Serial.println("Lid close detected, resuming PID");
    }

    // Detect sudden temperature drop
    if (!isnan(_prevPitTemp) && !isnan(_pitTemp) && _running) {
        float drop = _prevPitTemp - _pitTemp;
        if (drop >= LID_OPEN_DROP_C) {
            _lidOpen = true;
            _lidOpenUntil = now + LID_OPEN_PAUSE_MS;
            _fan.stop();
            Serial.printf("Lid open detected! Temp dropped %.1f°C\n", drop);
        }
    }
}

void SmokerController::recordHistory() {
    uint32_t now = millis();
    if (now - _lastHistoryWrite < TEMP_READ_INTERVAL_MS) return;
    _lastHistoryWrite = now;

    _history[_historyIdx] = {_pitTemp, _meatTemp, now};
    _historyIdx = (_historyIdx + 1) % HISTORY_SIZE;
    if (_historyCount < HISTORY_SIZE) _historyCount++;
}

void SmokerController::setPIDTunings(float kp, float ki, float kd) {
    _pid.setTunings(kp, ki, kd);
}

void SmokerController::setManualFanSpeed(float percent) {
    _manualFan = true;
    _manualFanSpeed = percent;
    _fan.setSpeed(percent);
}

void SmokerController::stop() {
    _running = false;
    _manualFan = false;
    _fan.stop();
    _pid.reset();
    _pidOutput = 0;
}
