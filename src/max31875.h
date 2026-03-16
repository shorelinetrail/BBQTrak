#pragma once

#include <Wire.h>
#include <cstdint>

class MAX31875 {
public:
    explicit MAX31875(uint8_t address);

    // Initialize sensor. Returns true on success.
    bool begin(TwoWire& wire = Wire);

    // Read temperature in °C. Returns NAN on failure.
    float readTempC();

    // Check if sensor is connected
    bool isConnected();

    // Configure resolution (9-12 bits). Higher = slower but more precise.
    // 9-bit: 25ms, 10-bit: 50ms, 11-bit: 100ms, 12-bit: 200ms
    void setResolution(uint8_t bits);

private:
    uint8_t _addr;
    TwoWire* _wire = nullptr;
    bool _connected = false;

    uint16_t readRegister16(uint8_t reg);
    void writeRegister16(uint8_t reg, uint16_t value);
};
