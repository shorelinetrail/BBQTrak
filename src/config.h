#pragma once

#include <cstdint>

// --- Smoker: Landmann Kentucky Offset ---
// Thin-walled, uninsulated offset with rotating disc damper on firebox.
// Fan mounts over the sealed damper opening; all other air leaks sealed.

// --- SPI Pins (VSPI) ---
// CLK and MISO shared between both MAX31855 chips
constexpr uint8_t PIN_SPI_CLK  = 18;
constexpr uint8_t PIN_SPI_MISO = 19;  // MAX31855 DO (data out)
// Separate CS pins for each thermocouple
constexpr uint8_t PIN_CS_PIT   = 5;   // Pit/chamber K-type thermocouple
constexpr uint8_t PIN_CS_MEAT  = 17;  // Meat K-type thermocouple

// --- Fan PWM ---
// Fan pushes air through sealed rotating damper opening — needs higher
// minimum duty and stronger kick-start to overcome the restriction.
constexpr uint8_t PIN_FAN_PWM = 25;       // GPIO25 -> 100R -> IRLZ44N gate
constexpr uint8_t FAN_PWM_CHANNEL = 0;
constexpr uint32_t FAN_PWM_FREQ = 25000;  // 25 kHz
constexpr uint8_t FAN_PWM_RESOLUTION = 8; // 0-255

// --- PID Defaults (tuned for Landmann Kentucky) ---
// Thin steel responds quickly to airflow changes — moderate Kp, low Ki
// to avoid windup from heat leaks, higher Kd to dampen oscillations.
constexpr float PID_KP_DEFAULT = 3.5f;
constexpr float PID_KI_DEFAULT = 0.012f;
constexpr float PID_KD_DEFAULT = 14.0f;
constexpr float PID_OUTPUT_MIN = 0.0f;
constexpr float PID_OUTPUT_MAX = 100.0f;

// --- Smoker Defaults ---
constexpr float DEFAULT_TARGET_TEMP_C = 121.0f;  // ~250°F
constexpr float MEAT_TARGET_TEMP_C    = 91.0f;   // ~195°F (brisket)

// --- Lid Open Detection ---
// Thin uninsulated walls lose heat fast — a smaller drop reliably
// indicates a lid open, and recovery takes longer.
constexpr float LID_OPEN_DROP_C       = 7.0f;    // °C drop to detect lid open
constexpr uint32_t LID_OPEN_PAUSE_MS  = 75000;   // Pause PID for 75s after lid open

// --- Fan Kick-Start ---
// Restricted damper opening needs a harder kick and longer pulse to
// build enough pressure to start moving air through the firebox.
constexpr uint8_t FAN_KICKSTART_DUTY  = 220;     // ~86% duty for kick-start
constexpr uint32_t FAN_KICKSTART_MS   = 700;     // 700ms kick-start duration
constexpr uint8_t FAN_MIN_DUTY        = 65;      // Minimum duty to push air through damper (~25%)

// --- Control Loop ---
constexpr uint32_t CONTROL_INTERVAL_MS = 2000;   // PID update every 2s
constexpr uint32_t TEMP_READ_INTERVAL_MS = 1000;  // Read sensors every 1s

// --- WiFi ---
// Credentials stored in NVS via WiFi provisioning portal
constexpr const char* HOSTNAME = "bbqtrak";
constexpr const char* AP_SSID  = "BBQTrak-Setup";

// --- Temperature History ---
constexpr size_t HISTORY_SIZE = 360;  // 6 minutes at 1s intervals
