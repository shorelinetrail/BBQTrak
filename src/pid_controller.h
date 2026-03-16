#pragma once

class PIDController {
public:
    PIDController(float kp, float ki, float kd,
                  float outputMin, float outputMax);

    // Compute PID output given current temperature and setpoint
    float compute(float setpoint, float currentTemp, float dt);

    // Reset integral and derivative state
    void reset();

    // Tune parameters
    void setTunings(float kp, float ki, float kd);
    float getKp() const { return _kp; }
    float getKi() const { return _ki; }
    float getKd() const { return _kd; }

    // Get last error for diagnostics
    float getLastError() const { return _lastError; }
    float getIntegral() const { return _integral; }

private:
    float _kp, _ki, _kd;
    float _outputMin, _outputMax;
    float _integral = 0.0f;
    float _lastError = 0.0f;
    bool _firstRun = true;
};
