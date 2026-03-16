#pragma once

#include <cstdint>

class FanControl {
public:
    void begin();

    // Set fan speed as percentage (0-100). Handles kick-start automatically.
    void setSpeed(float percent);

    // Get current speed percentage
    float getSpeed() const { return _speedPercent; }

    // Force fan off
    void stop();

    // Update must be called from loop() to manage kick-start timing
    void update();

private:
    float _speedPercent = 0.0f;
    uint8_t _currentDuty = 0;
    bool _kickStarting = false;
    uint32_t _kickStartEnd = 0;
    uint8_t _targetDuty = 0;
    bool _wasOff = true;

    void setDuty(uint8_t duty);
};
