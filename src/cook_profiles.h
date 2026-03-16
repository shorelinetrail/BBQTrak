#pragma once

#include <cstdint>

struct CookProfile {
    const char* name;
    float pitTargetC;     // Pit/chamber target °C
    float meatTargetC;    // Meat done target °C
    float kp, ki, kd;     // PID tunings for this cook style
    float rampDownC;      // Start ramp-down when meat is this many °C from target
};

// PID zone presets: low-and-slow needs gentler response, hot-and-fast more aggressive
enum PIDZone : uint8_t {
    PID_ZONE_LOW_SLOW = 0,   // up to 135°C
    PID_ZONE_HOT_FAST,       // 135°C+
    PID_ZONE_COUNT
};

constexpr CookProfile COOK_PROFILES[] = {
    //  name                pitC    meatC   kp     ki      kd     rampC
    { "Brisket",          107.0f, 96.0f,  3.0f,  0.015f, 12.0f, 5.5f  },  // 107°C pit, 96°C meat
    { "Pulled Pork",      107.0f, 96.0f,  3.0f,  0.015f, 12.0f, 5.5f  },  // 107°C pit, 96°C meat
    { "Ribs",             121.0f, 93.0f,  4.0f,  0.020f, 10.0f, 3.0f  },  // 121°C pit, 93°C meat
    { "Chicken",          163.0f, 74.0f,  5.0f,  0.025f, 8.0f,  3.0f  },  // 163°C pit, 74°C meat
    { "Hot & Fast Brisket", 149.0f, 96.0f, 5.5f, 0.030f, 8.0f,  5.5f },  // 149°C pit, 96°C meat
    { "Pork Belly",       121.0f, 91.0f,  4.0f,  0.020f, 10.0f, 3.0f  },  // 121°C pit, 91°C meat
    { "Turkey",           163.0f, 74.0f,  5.0f,  0.025f, 8.0f,  3.0f  },  // 163°C pit, 74°C meat
    { "Salmon",           107.0f, 63.0f,  3.0f,  0.015f, 12.0f, 2.0f  },  // 107°C pit, 63°C meat
};

constexpr size_t COOK_PROFILE_COUNT = sizeof(COOK_PROFILES) / sizeof(COOK_PROFILES[0]);

// Multi-zone PID presets (used when no cook profile is active)
struct PIDZonePreset {
    const char* name;
    float kp, ki, kd;
    float minPitC;  // Zone applies when pit target >= this
    float maxPitC;  // Zone applies when pit target < this
};

constexpr PIDZonePreset PID_ZONE_PRESETS[PID_ZONE_COUNT] = {
    { "Low & Slow",  3.0f, 0.015f, 12.0f,  0.0f,   135.0f },  // up to 135°C
    { "Hot & Fast",  5.5f, 0.030f,  8.0f,  135.0f,  999.0f },  // 135°C+
};
