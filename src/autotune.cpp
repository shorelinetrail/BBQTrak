#include "autotune.h"
#include <Arduino.h>
#include <math.h>

void PIDAutotuner::begin(float setpoint, float outputLow, float outputHigh) {
    _setpoint = setpoint;
    _outputLow = outputLow;
    _outputHigh = outputHigh;
    _running = true;
    _complete = false;
    _outputHigh_active = true;  // Start with high output to heat up

    _peakHigh = -999;
    _peakLow = 999;
    _lastPeakTime = 0;
    _elapsedTime = 0;
    _periodSum = 0;
    _amplitudeSum = 0;
    _cycleCount = 0;
    _lastAbove = false;
    _result = {};

    Serial.println("Autotune: started");
    Serial.printf("Autotune: setpoint=%.1f°C, output=[%.0f, %.0f]\n",
                  _setpoint, _outputLow, _outputHigh);
}

float PIDAutotuner::update(float currentTemp, float dt) {
    if (!_running || _complete) return _outputLow;

    _elapsedTime += dt;

    bool above = currentTemp > (_setpoint + HYSTERESIS);
    bool below = currentTemp < (_setpoint - HYSTERESIS);

    // Track peaks
    if (currentTemp > _peakHigh) _peakHigh = currentTemp;
    if (currentTemp < _peakLow) _peakLow = currentTemp;

    // Relay control: switch output at setpoint crossings
    if (above && _outputHigh_active) {
        // Crossed above setpoint - switch to low output
        _outputHigh_active = false;

        if (_lastAbove) {
            // Completed a full cycle (high->low->high)
            float period = _elapsedTime - _lastPeakTime;
            float amplitude = _peakHigh - _peakLow;

            if (period > 0 && amplitude > 0) {
                _periodSum += period;
                _amplitudeSum += amplitude;
                _cycleCount++;

                Serial.printf("Autotune: cycle %d, period=%.1fs, amplitude=%.2f°C\n",
                              _cycleCount, period, amplitude);
            }

            _peakHigh = currentTemp;
            _peakLow = currentTemp;
        }
        _lastPeakTime = _elapsedTime;
        _lastAbove = true;
    } else if (below && !_outputHigh_active) {
        // Crossed below setpoint - switch to high output
        _outputHigh_active = true;
    }

    // Check if we have enough cycles
    if (_cycleCount >= MIN_CYCLES) {
        _running = false;
        _complete = true;

        float tu = _periodSum / _cycleCount;             // Average period
        float amplitude = _amplitudeSum / _cycleCount;    // Average amplitude
        float ku = (4.0f * (_outputHigh - _outputLow)) / (M_PI * amplitude);  // Ultimate gain

        // Ziegler-Nichols PID tuning rules
        _result.ku = ku;
        _result.tu = tu;
        _result.kp = 0.6f * ku;
        _result.ki = 1.2f * ku / tu;
        _result.kd = 0.075f * ku * tu;
        _result.valid = true;

        Serial.println("Autotune: complete!");
        Serial.printf("Autotune: Ku=%.3f, Tu=%.1fs\n", ku, tu);
        Serial.printf("Autotune: Kp=%.3f, Ki=%.4f, Kd=%.3f\n",
                      _result.kp, _result.ki, _result.kd);
    } else if (_elapsedTime > 3600) {
        // Timeout after 1 hour
        _running = false;
        _complete = true;
        _result.valid = false;
        Serial.println("Autotune: timed out after 1 hour");
    }

    return _outputHigh_active ? _outputHigh : _outputLow;
}

void PIDAutotuner::cancel() {
    _running = false;
    _complete = false;
    _result.valid = false;
    Serial.println("Autotune: cancelled");
}
