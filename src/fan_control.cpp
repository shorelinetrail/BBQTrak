#include "fan_control.h"
#include "config.h"
#include <Arduino.h>

void FanControl::begin() {
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(PIN_FAN_PWM, FAN_PWM_CHANNEL);
    ledcWrite(FAN_PWM_CHANNEL, 0);
}

void FanControl::setSpeed(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    _speedPercent = percent;

    if (percent <= 0.0f) {
        _targetDuty = 0;
    } else {
        // Map 1-100% to FAN_MIN_DUTY-255 range so the fan always spins
        _targetDuty = FAN_MIN_DUTY + (uint8_t)((255 - FAN_MIN_DUTY) * percent / 100.0f);
    }

    // Kick-start: if fan was off and now needs to spin, pulse it briefly
    if (_wasOff && _targetDuty > 0) {
        _kickStarting = true;
        _kickStartEnd = millis() + FAN_KICKSTART_MS;
        setDuty(FAN_KICKSTART_DUTY);
    } else if (_targetDuty == 0) {
        _kickStarting = false;
        setDuty(0);
    } else if (!_kickStarting) {
        setDuty(_targetDuty);
    }

    _wasOff = (_targetDuty == 0);
}

void FanControl::stop() {
    _speedPercent = 0.0f;
    _targetDuty = 0;
    _kickStarting = false;
    _wasOff = true;
    setDuty(0);
}

void FanControl::update() {
    if (_kickStarting && millis() >= _kickStartEnd) {
        _kickStarting = false;
        setDuty(_targetDuty);
    }
}

void FanControl::setDuty(uint8_t duty) {
    _currentDuty = duty;
    ledcWrite(FAN_PWM_CHANNEL, duty);
}
