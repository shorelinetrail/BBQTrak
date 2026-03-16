#include "max31875.h"
#include "config.h"
#include <math.h>

MAX31875::MAX31875(uint8_t address) : _addr(address) {}

bool MAX31875::begin(TwoWire& wire) {
    _wire = &wire;
    _connected = isConnected();
    if (_connected) {
        setResolution(12);  // 12-bit for best accuracy (0.0625°C)
    }
    return _connected;
}

bool MAX31875::isConnected() {
    if (!_wire) return false;
    _wire->beginTransmission(_addr);
    return (_wire->endTransmission() == 0);
}

float MAX31875::readTempC() {
    if (!_wire) return NAN;

    uint16_t raw = readRegister16(MAX31875_REG_TEMP);
    if (raw == 0xFFFF) return NAN;

    // MAX31875 temperature register format:
    // MSB: Sign + 7 integer bits
    // LSB: fractional bits (depends on resolution)
    // Value is in two's complement, left-justified
    int16_t temp = static_cast<int16_t>(raw);
    return temp / 256.0f;  // 12-bit: divide by 256 to get °C
}

void MAX31875::setResolution(uint8_t bits) {
    if (bits < 9) bits = 9;
    if (bits > 12) bits = 12;

    uint16_t config = readRegister16(MAX31875_REG_CONFIG);
    // Resolution bits are in config register bits [10:9]
    config &= ~(0x03 << 9);             // Clear resolution bits
    config |= ((bits - 9) & 0x03) << 9; // Set new resolution
    writeRegister16(MAX31875_REG_CONFIG, config);
}

uint16_t MAX31875::readRegister16(uint8_t reg) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (_wire->endTransmission() != 0) return 0xFFFF;

    if (_wire->requestFrom(_addr, (uint8_t)2) != 2) return 0xFFFF;

    uint16_t value = _wire->read() << 8;
    value |= _wire->read();
    return value;
}

void MAX31875::writeRegister16(uint8_t reg, uint16_t value) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(value >> 8);
    _wire->write(value & 0xFF);
    _wire->endTransmission();
}
