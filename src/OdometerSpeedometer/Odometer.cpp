#include "Odometer.h"
#include "configuration.h"
#include <limits>

namespace
{
constexpr uint64_t SAVE_INTERVAL_PULSES =
    static_cast<uint64_t>(PULSES_PER_MILE) * ODOMETER_SAVE_INTERVAL_MILES;
constexpr char PREFERENCES_NAMESPACE[] = "odometer";
constexpr char PULSE_COUNT_KEY[] = "pulses";
}

bool Odometer::begin()
{
    persistenceAvailable = preferences.begin(PREFERENCES_NAMESPACE, false);
    if (!persistenceAvailable)
    {
        Serial.println("ERROR: Could not open NVS for odometer storage.");
        return false;
    }

    totalPulseCount = preferences.getULong64(PULSE_COUNT_KEY, 0);
    lastPersistedPulseCount = totalPulseCount;
    return true;
}

void Odometer::incrementPulseCount()
{
    pendingPulseCount.fetch_add(1, std::memory_order_relaxed);
}

void Odometer::update()
{
    const uint32_t newPulses =
        pendingPulseCount.exchange(0, std::memory_order_relaxed);
    if (std::numeric_limits<uint64_t>::max() - totalPulseCount < newPulses)
    {
        totalPulseCount = std::numeric_limits<uint64_t>::max();
    }
    else
    {
        totalPulseCount += newPulses;
    }

    if (!persistenceAvailable ||
        totalPulseCount - lastPersistedPulseCount < SAVE_INTERVAL_PULSES)
    {
        return;
    }

    const uint32_t now = millis();
    if (persistenceAttempted &&
        now - lastPersistenceAttemptMillis < ODOMETER_SAVE_RETRY_INTERVAL_MS)
    {
        return;
    }

    persistenceAttempted = true;
    lastPersistenceAttemptMillis = now;
    const size_t bytesWritten =
        preferences.putULong64(PULSE_COUNT_KEY, totalPulseCount);
    if (bytesWritten != sizeof(totalPulseCount))
    {
        Serial.println("ERROR: Failed to persist odometer pulse count to NVS.");
        return;
    }

    lastPersistedPulseCount = totalPulseCount;
}

uint32_t Odometer::getHundredthsOfMiles() const
{
    const uint64_t wholeMiles = totalPulseCount / PULSES_PER_MILE;
    const uint64_t partialMilePulses = totalPulseCount % PULSES_PER_MILE;
    const uint64_t hundredths =
        wholeMiles * 100 + partialMilePulses * 100 / PULSES_PER_MILE;
    return hundredths > std::numeric_limits<uint32_t>::max()
               ? std::numeric_limits<uint32_t>::max()
               : static_cast<uint32_t>(hundredths);
}