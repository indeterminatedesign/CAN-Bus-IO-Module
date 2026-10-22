#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <atomic>

class Odometer
{
public:
    bool begin();
    void incrementPulseCount();
    void update();
    uint32_t getHundredthsOfMiles() const;

private:
    Preferences preferences;
    std::atomic<uint32_t> pendingPulseCount{0};
    uint64_t totalPulseCount = 0;
    uint64_t lastPersistedPulseCount = 0;
    uint32_t lastPersistenceAttemptMillis = 0;
    bool persistenceAvailable = false;
    bool persistenceAttempted = false;
};