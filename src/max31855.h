#pragma once

#include <SPI.h>
#include <cstdint>

class MAX31855 {
public:
    explicit MAX31855(uint8_t csPin);

    // Initialize sensor. Call after SPI.begin().
    void begin();

    // Read thermocouple temperature in °C. Returns NAN on fault.
    float readTempC();

    // Read cold-junction (internal) temperature in °C.
    float readInternalC();

    // Check if thermocouple is connected (no fault bits set)
    bool isConnected();

    // Get fault code from last read (0 = no fault)
    // Bit 0: Open circuit, Bit 1: Short to GND, Bit 2: Short to VCC
    uint8_t getFault() const { return _fault; }

private:
    uint8_t _csPin;
    uint8_t _fault = 0;
    uint32_t _lastRaw = 0;

    uint32_t readRaw();
};
