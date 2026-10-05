#include "FuelGauge.h"

FuelGauge::FuelGauge(MCP3204& adc, uint8_t channel, float alpha) 
    : _adc(adc), _adcChannel(channel), _lastFuelLevel(0.0f), _filterAlpha(alpha) 
{
}

void FuelGauge::begin() 
{
    // Take an initial baseline reading to prevent filter startup lag
    _lastFuelLevel = (float)_adc.read(_adcChannel);
}

float FuelGauge::update() 
{
    // 1. Read raw voltage value from external hardware SPI ADC
    uint16_t rawAdc = _adc.read(_adcChannel);
    
    // 2. Smooth signal ripples using Exponential Moving Average
    float filtered = emaFilter((float)rawAdc, _lastFuelLevel, _filterAlpha);
    _lastFuelLevel = filtered;
    
    // 3. Scale raw 12-bit hardware bounds to standard automotive percentage layout
    return getLevel();
}

float FuelGauge::getLevel() const 
{
    // Scale 0-4095 over a 0-100% boundary limit
    return constrain((_lastFuelLevel / 4095.0f) * 100.0f, 0.0f, 100.0f);
}

float FuelGauge::emaFilter(float newValue, float prevEma, float alpha) 
{
    return (alpha * newValue) + ((1.0f - alpha) * prevEma);
}
