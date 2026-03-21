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

    // Auto-select PID zone based on default target
    autoSelectPIDZone();
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
        computeRampDown();
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

    // Periodic debug output every 10 seconds
    _debugCounter++;
    if (_debugCounter >= 10) {
        _debugCounter = 0;
        Serial.printf("[SENSORS] pit=%.1f°C meat=%.1f°C | pit_fault=%d meat_fault=%d | target=%.1f°C fan=%.0f%% pid=%.1f %s\n",
                      _pitTemp, _meatTemp,
                      _pitProbe.getFault(), _meatProbe.getFault(),
                      _effectiveTarget, _fan.getSpeed(), _pidOutput,
                      _running ? "RUN" : "STOP");
    }
}

void SmokerController::computeRampDown() {
    _rampingDown = false;
    _effectiveTarget = _targetTemp;

    // Only ramp down if meat probe is connected and reading valid
    if (isnan(_meatTemp) || isnan(_pitTemp)) return;

    float distToTarget = _meatTarget - _meatTemp;

    // When meat is within ramp-down distance of its target,
    // progressively lower the pit target to prevent overshoot
    // from carry-over cooking (meat continues to rise after pulling)
    if (distToTarget > 0 && distToTarget <= _rampDownC) {
        _rampingDown = true;

        // Linear ramp: as meat approaches target, reduce pit temp toward meat target
        // At rampDownC away: full pit target
        // At 0 away: pit target drops to meatTarget (just maintaining)
        float rampFactor = distToTarget / _rampDownC;  // 1.0 -> 0.0
        _effectiveTarget = _meatTarget + (_targetTemp - _meatTarget) * rampFactor;

        // Don't let effective target go below meat target
        if (_effectiveTarget < _meatTarget) {
            _effectiveTarget = _meatTarget;
        }
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

    // If autotuning, use autotuner output instead of PID
    if (_autotuner.isRunning()) {
        _pidOutput = _autotuner.update(_pitTemp, dt);
        _fan.setSpeed(_pidOutput);
        return;
    }

    // Use effective target (may be reduced by ramp-down)
    _pidOutput = _pid.compute(_effectiveTarget, _pitTemp, dt);
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

// --- Cook Profiles ---

void SmokerController::setProfile(int8_t index) {
    if (index < 0 || index >= (int8_t)COOK_PROFILE_COUNT) {
        _activeProfile = -1;
        return;
    }

    _activeProfile = index;
    const CookProfile& p = COOK_PROFILES[index];
    _targetTemp = p.pitTargetC;
    _effectiveTarget = p.pitTargetC;
    _meatTarget = p.meatTargetC;
    _rampDownC = p.rampDownC;
    _pid.setTunings(p.kp, p.ki, p.kd);
    _pid.reset();

    Serial.printf("Profile: %s (pit=%.0f°C, meat=%.0f°C, Kp=%.2f Ki=%.3f Kd=%.2f)\n",
                  p.name, p.pitTargetC, p.meatTargetC, p.kp, p.ki, p.kd);
}

// --- Multi-Zone PID ---

void SmokerController::autoSelectPIDZone() {
    for (uint8_t i = 0; i < PID_ZONE_COUNT; i++) {
        const PIDZonePreset& z = PID_ZONE_PRESETS[i];
        if (_targetTemp >= z.minPitC && _targetTemp < z.maxPitC) {
            if (_activePIDZone != i) {
                _activePIDZone = i;
                // Only auto-apply if no cook profile is active
                if (_activeProfile < 0) {
                    _pid.setTunings(z.kp, z.ki, z.kd);
                    Serial.printf("PID zone auto-selected: %s (Kp=%.2f Ki=%.3f Kd=%.2f)\n",
                                  z.name, z.kp, z.ki, z.kd);
                }
            }
            return;
        }
    }
}

const char* SmokerController::getActivePIDZoneName() const {
    if (_activeProfile >= 0) {
        return COOK_PROFILES[_activeProfile].name;
    }
    if (_activePIDZone >= 0 && _activePIDZone < PID_ZONE_COUNT) {
        return PID_ZONE_PRESETS[_activePIDZone].name;
    }
    return "Custom";
}

void SmokerController::setTargetTemp(float tempC) {
    _targetTemp = tempC;
    _effectiveTarget = tempC;
    // Auto-select PID zone when target changes (only if no profile active)
    if (_activeProfile < 0) {
        autoSelectPIDZone();
    }
}

// --- Autotune ---

void SmokerController::startAutotune() {
    if (!_running) {
        _running = true;
    }
    _manualFan = false;
    _autotuner.begin(_targetTemp, 0.0f, 100.0f);
}

void SmokerController::cancelAutotune() {
    _autotuner.cancel();
}

void SmokerController::applyAutotuneResult() {
    PIDAutotuner::Result r = _autotuner.getResult();
    if (r.valid) {
        _pid.setTunings(r.kp, r.ki, r.kd);
        _pid.reset();
        Serial.printf("Applied autotune: Kp=%.3f Ki=%.4f Kd=%.3f\n", r.kp, r.ki, r.kd);
    }
}

// --- General ---

void SmokerController::setPIDTunings(float kp, float ki, float kd) {
    _pid.setTunings(kp, ki, kd);
    _activeProfile = -1;  // Custom tuning overrides profile
}

void SmokerController::setManualFanSpeed(float percent) {
    _manualFan = true;
    _manualFanSpeed = percent;
    _fan.setSpeed(percent);
}

void SmokerController::stop() {
    _running = false;
    _manualFan = false;
    _rampingDown = false;
    _fan.stop();
    _pid.reset();
    _pidOutput = 0;
    if (_autotuner.isRunning()) _autotuner.cancel();
}
