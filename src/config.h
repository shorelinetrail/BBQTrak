#pragma once

#include <cstdint>

// --- I2C Pins ---
constexpr uint8_t PIN_SDA = 21;
constexpr uint8_t PIN_SCL = 22;

// --- Fan PWM ---
constexpr uint8_t PIN_FAN_PWM = 25;       // GPIO25 -> 100R -> IRLZ44N gate
constexpr uint8_t FAN_PWM_CHANNEL = 0;
constexpr uint32_t FAN_PWM_FREQ = 25000;  // 25 kHz
constexpr uint8_t FAN_PWM_RESOLUTION = 8; // 0-255

// --- MAX31875 I2C Addresses ---
// A0-A2 configure address: 0x48 + (A2<<2 | A1<<1 | A0)
constexpr uint8_t MAX31875_PIT_ADDR  = 0x48;  // Pit/chamber probe
constexpr uint8_t MAX31875_MEAT_ADDR = 0x49;  // Meat probe

// --- MAX31875 Registers ---
constexpr uint8_t MAX31875_REG_TEMP   = 0x00;
constexpr uint8_t MAX31875_REG_CONFIG = 0x01;

// --- PID Defaults ---
constexpr float PID_KP_DEFAULT = 4.0f;
constexpr float PID_KI_DEFAULT = 0.02f;
constexpr float PID_KD_DEFAULT = 10.0f;
constexpr float PID_OUTPUT_MIN = 0.0f;
constexpr float PID_OUTPUT_MAX = 100.0f;

// --- Smoker Defaults ---
constexpr float DEFAULT_TARGET_TEMP_C = 121.0f;  // ~250°F
constexpr float MEAT_TARGET_TEMP_C    = 91.0f;   // ~195°F (brisket)

// --- Lid Open Detection ---
constexpr float LID_OPEN_DROP_C       = 10.0f;   // °C drop to detect lid open
constexpr uint32_t LID_OPEN_PAUSE_MS  = 60000;   // Pause PID for 60s after lid open

// --- Fan Kick-Start ---
constexpr uint8_t FAN_KICKSTART_DUTY  = 200;     // ~78% duty for kick-start
constexpr uint32_t FAN_KICKSTART_MS   = 500;     // 500ms kick-start duration
constexpr uint8_t FAN_MIN_DUTY        = 50;      // Minimum duty to spin fan (~20%)

// --- Control Loop ---
constexpr uint32_t CONTROL_INTERVAL_MS = 2000;   // PID update every 2s
constexpr uint32_t TEMP_READ_INTERVAL_MS = 1000;  // Read sensors every 1s

// --- WiFi ---
// Set your WiFi credentials here or via the web interface on first boot
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char* WIFI_PASS = "YOUR_WIFI_PASS";
constexpr const char* HOSTNAME  = "bbqtrak";

// --- Temperature History ---
constexpr size_t HISTORY_SIZE = 360;  // 6 minutes at 1s intervals
