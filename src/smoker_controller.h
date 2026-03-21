#pragma once

#include <cmath>
#include "max31855.h"
#include "fan_control.h"
#include "pid_controller.h"
#include "autotune.h"
#include "cook_profiles.h"
#include "config.h"

struct TempReading {
    float pitC;
    float meatC;
    uint32_t timestamp;
};

class SmokerController {
public:
    SmokerController();

    void begin();
    void update();

    // Getters
    float getPitTempC() const { return _pitTemp; }
    float getMeatTempC() const { return _meatTemp; }
    float getTargetTempC() const { return _targetTemp; }
    float getMeatTargetC() const { return _meatTarget; }
    float getEffectiveTargetC() const { return _effectiveTarget; }
    float getFanSpeed() const { return _fan.getSpeed(); }
    float getPidOutput() const { return _pidOutput; }
    bool isLidOpen() const { return _lidOpen; }
    bool isPitProbeConnected() const { return _pitProbe.isConnected(); }
    bool isMeatProbeConnected() const { return _meatProbe.isConnected(); }
    bool isRunning() const { return _running; }
    bool isRampingDown() const { return _rampingDown; }
    const PIDController& getPID() const { return _pid; }

    // Cook profile
    int8_t getActiveProfile() const { return _activeProfile; }
    void setProfile(int8_t index);  // -1 = custom/none

    // Autotune
    bool isAutotuning() const { return _autotuner.isRunning(); }
    bool isAutotuneComplete() const { return _autotuner.isComplete(); }
    PIDAutotuner::Result getAutotuneResult() const { return _autotuner.getResult(); }
    void startAutotune();
    void cancelAutotune();
    void applyAutotuneResult();

    // Multi-zone PID
    const char* getActivePIDZoneName() const;
    void autoSelectPIDZone();  // Auto-select based on pit target

    // History access
    const TempReading* getHistory() const { return _history; }
    size_t getHistoryCount() const { return _historyCount; }
    size_t getHistoryIndex() const { return _historyIdx; }

    // Setters
    void setTargetTemp(float tempC);
    void setMeatTarget(float tempC) { _meatTarget = tempC; }
    void setPIDTunings(float kp, float ki, float kd);
    void setManualFanSpeed(float percent);
    void setAutoMode() { _manualFan = false; }
    void start() { _running = true; _pid.reset(); }
    void stop();

private:
    MAX31855 _pitProbe;
    MAX31855 _meatProbe;
    FanControl _fan;
    PIDController _pid;
    PIDAutotuner _autotuner;

    float _pitTemp = NAN;
    float _meatTemp = NAN;
    float _targetTemp = DEFAULT_TARGET_TEMP_C;
    float _effectiveTarget = DEFAULT_TARGET_TEMP_C;
    float _meatTarget = MEAT_TARGET_TEMP_C;
    float _rampDownC = 5.5f;  // Start ramp-down when meat is this close to target
    float _pidOutput = 0.0f;
    bool _lidOpen = false;
    bool _running = false;
    bool _manualFan = false;
    bool _rampingDown = false;
    float _manualFanSpeed = 0.0f;
    int8_t _activeProfile = -1;
    int8_t _activePIDZone = -1;

    uint32_t _lastTempRead = 0;
    uint32_t _lastControl = 0;
    uint32_t _lidOpenUntil = 0;
    float _prevPitTemp = NAN;
    uint8_t _debugCounter = 0;

    // Circular buffer for history
    TempReading _history[HISTORY_SIZE];
    size_t _historyIdx = 0;
    size_t _historyCount = 0;
    uint32_t _lastHistoryWrite = 0;

    void readSensors();
    void runControl();
    void detectLidOpen();
    void recordHistory();
    void computeRampDown();
};
