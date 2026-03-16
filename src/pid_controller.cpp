#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd,
                             float outputMin, float outputMax)
    : _kp(kp), _ki(ki), _kd(kd),
      _outputMin(outputMin), _outputMax(outputMax) {}

float PIDController::compute(float setpoint, float currentTemp, float dt) {
    if (dt <= 0.0f) return 0.0f;

    float error = setpoint - currentTemp;

    // Proportional
    float pTerm = _kp * error;

    // Integral with anti-windup clamping
    _integral += error * dt;
    float iTerm = _ki * _integral;

    // Clamp integral to prevent windup
    if (iTerm > _outputMax) {
        _integral = _outputMax / _ki;
        iTerm = _outputMax;
    } else if (iTerm < _outputMin) {
        _integral = _outputMin / _ki;
        iTerm = _outputMin;
    }

    // Derivative (on error, with first-run guard)
    float dTerm = 0.0f;
    if (!_firstRun) {
        dTerm = _kd * (error - _lastError) / dt;
    }
    _firstRun = false;
    _lastError = error;

    // Sum and clamp output
    float output = pTerm + iTerm + dTerm;
    if (output > _outputMax) output = _outputMax;
    if (output < _outputMin) output = _outputMin;

    // Hard cutoff when above setpoint to prevent overshoot
    if (error < -1.0f) {
        output = 0.0f;
    }

    return output;
}

void PIDController::reset() {
    _integral = 0.0f;
    _lastError = 0.0f;
    _firstRun = true;
}

void PIDController::setTunings(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}
