#include "max31855.h"
#include <math.h>

MAX31855::MAX31855(uint8_t csPin) : _csPin(csPin) {}

void MAX31855::begin() {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);  // Deselect

    // Do an initial read to populate fault status
    readRaw();
    if (_fault) {
        Serial.printf("[MAX31855 CS%d] INIT FAULT: raw=0x%08X fault=0x%02X (%s%s%s)\n",
                      _csPin, _lastRaw, _fault,
                      (_fault & 0x01) ? "OPEN " : "",
                      (_fault & 0x02) ? "SHORT_GND " : "",
                      (_fault & 0x04) ? "SHORT_VCC" : "");
    } else {
        float tc = readTempC();
        Serial.printf("[MAX31855 CS%d] INIT OK: raw=0x%08X temp=%.2f°C\n",
                      _csPin, _lastRaw, tc);
    }
}

float MAX31855::readTempC() {
    uint32_t raw = readRaw();
    if (_fault) {
        _consecutiveFaults++;
        // Log every fault for the first 5, then every 10th
        if (_consecutiveFaults <= 5 || (_consecutiveFaults % 10) == 0) {
            Serial.printf("[MAX31855 CS%d] FAULT #%u: raw=0x%08X fault=0x%02X (%s%s%s)\n",
                          _csPin, _consecutiveFaults, raw, _fault,
                          (_fault & 0x01) ? "OPEN " : "",
                          (_fault & 0x02) ? "SHORT_GND " : "",
                          (_fault & 0x04) ? "SHORT_VCC" : "");
        }
        return NAN;
    }

    _consecutiveFaults = 0;

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

bool MAX31855::isConnected() const {
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

    // Detect all-zeros or all-ones (no chip responding on SPI bus)
    if (raw == 0x00000000 || raw == 0xFFFFFFFF) {
        Serial.printf("[MAX31855 CS%d] SPI ERROR: raw=0x%08X — chip not responding, check wiring\n",
                      _csPin, raw);
        _fault = 0x07;  // Flag as faulted
        return raw;
    }

    // Fault bit is bit 16; fault details in bits [2:0]
    if (raw & 0x00010000) {
        _fault = raw & 0x07;
    } else {
        _fault = 0;
    }

    return raw;
}
