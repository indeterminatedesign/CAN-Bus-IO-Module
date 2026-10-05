#pragma once
#include <Arduino.h>
#include "configuration.h"
#include <atomic>

class Speedometer
{
public:
    float getSpeed();
    void incrementPulseCount();

private:
    uint32_t lastSpeedCalculationMillis = 0;
    uint16_t speed = 0;
    std::atomic<uint32_t> vssPulseCount = 0;
};
