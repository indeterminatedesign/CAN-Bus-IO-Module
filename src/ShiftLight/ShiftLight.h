#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "configuration.h"

class ShiftLight
{
public:
    void begin();
    void enable();
    void disable();
    void updateRPM(uint16_t rpm);
    void setShiftLightRPMBins(uint16_t startRpm, uint16_t shiftRpm, uint16_t maxRpm);

private:
    CRGB leds[NUM_LEDS];
    // Global timer state for the flashing over-rev animation
    unsigned long lastFlashTime = 0;
    bool flashState = false;
    uint16_t _startRpm = 4000; // Default starting RPM for the first LED
    uint16_t _shiftRpm = 6500; // Default RPM for the last LED (ideal shift point)
    uint16_t _maxRpm = 6800;   // Default RPM for over-rev warning (flash all LEDs)
    bool enabled = true; // Flag to enable or disable the shift light functionality
};