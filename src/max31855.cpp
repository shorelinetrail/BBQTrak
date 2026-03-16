#include "max31855.h"
#include <math.h>

MAX31855::MAX31855(uint8_t csPin) : _csPin(csPin) {}

void MAX31855::begin() {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);  // Deselect
}

float MAX31855::readTempC() {
    uint32_t raw = readRaw();
    if (_fault) return NAN;

    // Thermocouple temp is in bits [31:18] (14-bit signed)
    int16_t tc = (raw >> 18) & 0x3FFF;
    if (tc & 0x2000) {
        tc |= 0xC000;  // Sign-extend for negative temps
    }
    return tc * 0.25f;
}

float MAX31855::readInternalC() {
    uint32_t raw = readRaw();

    // Internal (cold-junction) temp is in bits [15:4] (12-bit signed)
    int16_t internal = (raw >> 4) & 0x0FFF;
    if (internal & 0x0800) {
        internal |= 0xF000;  // Sign-extend
    }
    return internal * 0.0625f;
}

bool MAX31855::isConnected() {
    readRaw();
    return (_fault == 0);
}

uint32_t MAX31855::readRaw() {
    // MAX31855 outputs 32 bits on each SPI read, MSB first
    // SPI Mode 0, max 5 MHz
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);

    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    uint32_t raw = 0;
    raw  = (uint32_t)SPI.transfer(0) << 24;
    raw |= (uint32_t)SPI.transfer(0) << 16;
    raw |= (uint32_t)SPI.transfer(0) << 8;
    raw |= (uint32_t)SPI.transfer(0);
    SPI.endTransaction();

    digitalWrite(_csPin, HIGH);

    _lastRaw = raw;

    // Fault bit is bit 16; fault details in bits [2:0]
    if (raw & 0x00010000) {
        _fault = raw & 0x07;
    } else {
        _fault = 0;
    }

    return raw;
}
