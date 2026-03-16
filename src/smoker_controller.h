#pragma once

#include "max31875.h"
#include "fan_control.h"
#include "pid_controller.h"
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
    float getFanSpeed() const { return _fan.getSpeed(); }
    float getPidOutput() const { return _pidOutput; }
    bool isLidOpen() const { return _lidOpen; }
    bool isPitProbeConnected() const { return _pitProbe.isConnected(); }
    bool isMeatProbeConnected() const { return _meatProbe.isConnected(); }
    bool isRunning() const { return _running; }
    const PIDController& getPID() const { return _pid; }

    // History access
    const TempReading* getHistory() const { return _history; }
    size_t getHistoryCount() const { return _historyCount; }
    size_t getHistoryIndex() const { return _historyIdx; }

    // Setters
    void setTargetTemp(float tempC) { _targetTemp = tempC; }
    void setMeatTarget(float tempC) { _meatTarget = tempC; }
    void setPIDTunings(float kp, float ki, float kd);
    void setManualFanSpeed(float percent);
    void setAutoMode() { _manualFan = false; }
    void start() { _running = true; _pid.reset(); }
    void stop();

private:
    MAX31875 _pitProbe;
    MAX31875 _meatProbe;
    FanControl _fan;
    PIDController _pid;

    float _pitTemp = NAN;
    float _meatTemp = NAN;
    float _targetTemp = DEFAULT_TARGET_TEMP_C;
    float _meatTarget = MEAT_TARGET_TEMP_C;
    float _pidOutput = 0.0f;
    bool _lidOpen = false;
    bool _running = false;
    bool _manualFan = false;
    float _manualFanSpeed = 0.0f;

    uint32_t _lastTempRead = 0;
    uint32_t _lastControl = 0;
    uint32_t _lidOpenUntil = 0;
    float _prevPitTemp = NAN;

    // Circular buffer for history
    TempReading _history[HISTORY_SIZE];
    size_t _historyIdx = 0;
    size_t _historyCount = 0;
    uint32_t _lastHistoryWrite = 0;

    void readSensors();
    void runControl();
    void detectLidOpen();
    void recordHistory();
};
