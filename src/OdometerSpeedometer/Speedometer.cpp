#include "Speedometer.h"

void Speedometer::incrementPulseCount()
{
    vssPulseCount.fetch_add(1, std::memory_order_relaxed);
}

float Speedometer::getSpeed()
{
    uint32_t now = millis();
    uint32_t deltaTime = now - lastSpeedCalculationMillis;

    // Only process if enough time has passed
    if (deltaTime > SPEED_CALCULATION_INTERVAL_MS)
    {
        // ATOMIC EXCHANGE: Reads the current count AND sets it to 0 
        // in a single, un-interruptible clock cycle
        uint32_t currentCount = vssPulseCount.exchange(0, std::memory_order_relaxed);

        if (currentCount == 0)
        {
            speed = 0;
        }
        else
        {
            // Calculate the speed in mph using the cleanly grabbed currentCount
            float pulsesPerMillisecond = (float)currentCount / (float)deltaTime;
            float pulsesPerHour = pulsesPerMillisecond * 3600000.0;

            speed = (uint16_t)(pulsesPerHour / (float)PULSES_PER_MILE);
        }
        
        // Update the timestamp ONLY when a calculation actually occurs
        lastSpeedCalculationMillis = now;
    }

    return speed;
}

