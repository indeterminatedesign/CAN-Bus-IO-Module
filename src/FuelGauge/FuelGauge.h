#pragma once
#include <Arduino.h>
#include "MCP_ADC.h"

class FuelGauge
{
public:
    /**
     * @param adc Reference to your existing initialized MCP3204 object.
     * @param channel The analog hardware channel the fuel sender is on.
     * @param alpha The smoothing weight factor (lower numbers = smoother/slower response).
     */
    FuelGauge(MCP3204 &adc, uint8_t channel, float alpha = 0.001f);

    // Initial baseline calibration read during setup
    void begin();

    // Runs a fresh hardware conversion read and updates the filter history
    float update();

    // Fast read-only query to get the last computed level without reading the hardware again
    float getLevel() const;

private:
    MCP3204 &_adc;
    uint8_t _adcChannel;
    float _lastFuelLevel;
    float _filterAlpha;

    // Internal signal smoothing math helper
    float emaFilter(float newValue, float prevEma, float alpha);
};
