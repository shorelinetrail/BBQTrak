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
    PID_ZONE_LOW_SLOW = 0,   // 200-275°F (93-135°C)
    PID_ZONE_HOT_FAST,       // 275-400°F (135-204°C)
    PID_ZONE_COUNT
};

constexpr CookProfile COOK_PROFILES[] = {
    //  name              pitC    meatC   kp     ki      kd     rampC
    { "Brisket",          107.2f, 96.1f,  3.0f,  0.015f, 12.0f, 5.5f  },  // 225°F pit, 205°F meat
    { "Pulled Pork",      107.2f, 96.1f,  3.0f,  0.015f, 12.0f, 5.5f  },  // 225°F pit, 205°F meat
    { "Ribs",             121.1f, 93.3f,  4.0f,  0.020f, 10.0f, 3.0f  },  // 250°F pit, 200°F meat
    { "Chicken",          162.8f, 73.9f,  5.0f,  0.025f, 8.0f,  3.0f  },  // 325°F pit, 165°F meat
    { "Hot & Fast Brisket", 148.9f, 96.1f, 5.5f, 0.030f, 8.0f,  5.5f },  // 300°F pit, 205°F meat
    { "Pork Belly",       121.1f, 90.6f,  4.0f,  0.020f, 10.0f, 3.0f  },  // 250°F pit, 195°F meat
    { "Turkey",           162.8f, 73.9f,  5.0f,  0.025f, 8.0f,  3.0f  },  // 325°F pit, 165°F meat
    { "Salmon",           107.2f, 62.8f,  3.0f,  0.015f, 12.0f, 2.0f  },  // 225°F pit, 145°F meat
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
    { "Low & Slow",  3.0f, 0.015f, 12.0f,  0.0f,   135.0f },  // up to ~275°F
    { "Hot & Fast",  5.5f, 0.030f,  8.0f,  135.0f,  999.0f },  // above ~275°F
};
