#pragma once

#include <cstdint>

// Relay-based PID autotuner using Ziegler-Nichols method.
// Oscillates output between 0% and 100% around a setpoint,
// measures the natural oscillation period and amplitude,
// then computes PID gains.
class PIDAutotuner {
public:
    struct Result {
        float kp, ki, kd;
        float ku;       // Ultimate gain
        float tu;       // Ultimate period (seconds)
        bool valid;
    };

    // Configure autotune around a setpoint with given output bounds
    void begin(float setpoint, float outputLow = 0.0f, float outputHigh = 100.0f);

    // Call each control cycle. Returns the output to apply to the fan.
    // When done, isComplete() returns true and getResult() has the tunings.
    float update(float currentTemp, float dt);

    bool isRunning() const { return _running; }
    bool isComplete() const { return _complete; }
    Result getResult() const { return _result; }

    // Cancel autotune
    void cancel();

private:
    static constexpr int MIN_CYCLES = 5;
    static constexpr int MAX_CYCLES = 20;
    static constexpr float HYSTERESIS = 0.5f;  // °C band around setpoint

    bool _running = false;
    bool _complete = false;
    float _setpoint = 0;
    float _outputLow = 0;
    float _outputHigh = 100;
    bool _outputHigh_active = false;

    // Oscillation tracking
    float _peakHigh = -999;
    float _peakLow = 999;
    float _lastPeakTime = 0;
    float _elapsedTime = 0;

    // Cycle measurement
    float _periodSum = 0;
    float _amplitudeSum = 0;
    int _cycleCount = 0;
    bool _lastAbove = false;

    Result _result = {};
};
