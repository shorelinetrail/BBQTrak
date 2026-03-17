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

// Tuned for Landmann Kentucky offset smoker:
// - Thin uninsulated steel responds fast → lower Ki to prevent integral windup
// - High heat loss through walls → higher Kd to dampen temp swings
// - Wider ramp-down margins: thin walls don't retain heat, so carry-over
//   cooking is less aggressive but the pit cools quickly once fan slows
constexpr CookProfile COOK_PROFILES[] = {
    //  name                pitC    meatC   kp     ki      kd     rampC
    { "Brisket",          107.0f, 96.0f,  2.8f,  0.010f, 15.0f, 4.5f  },
    { "Pulled Pork",      107.0f, 96.0f,  2.8f,  0.010f, 15.0f, 4.5f  },
    { "Ribs",             121.0f, 93.0f,  3.5f,  0.012f, 14.0f, 3.0f  },
    { "Chicken",          163.0f, 74.0f,  4.5f,  0.018f, 10.0f, 2.5f  },
    { "Hot & Fast Brisket", 149.0f, 96.0f, 5.0f, 0.022f, 10.0f, 4.5f },
    { "Pork Belly",       121.0f, 91.0f,  3.5f,  0.012f, 14.0f, 3.0f  },
    { "Turkey",           163.0f, 74.0f,  4.5f,  0.018f, 10.0f, 2.5f  },
    { "Salmon",           107.0f, 63.0f,  2.8f,  0.010f, 15.0f, 2.0f  },
};

constexpr size_t COOK_PROFILE_COUNT = sizeof(COOK_PROFILES) / sizeof(COOK_PROFILES[0]);

// Multi-zone PID presets (used when no cook profile is active)
// Tuned for Landmann Kentucky: lower Ki, higher Kd across both zones
struct PIDZonePreset {
    const char* name;
    float kp, ki, kd;
    float minPitC;  // Zone applies when pit target >= this
    float maxPitC;  // Zone applies when pit target < this
};

constexpr PIDZonePreset PID_ZONE_PRESETS[PID_ZONE_COUNT] = {
    { "Low & Slow",  2.8f, 0.010f, 15.0f,  0.0f,   135.0f },  // up to 135°C
    { "Hot & Fast",  5.0f, 0.022f, 10.0f,  135.0f,  999.0f },  // 135°C+
};
