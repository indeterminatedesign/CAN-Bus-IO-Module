#include "ShiftLight.h"

void ShiftLight::begin()
{
    // Pin has to be known at compile time so must be defined in configuration.h
    FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(160); // Set overall brightness (0 - 255)
    FastLED.clear(true);
}

void ShiftLight::enable()
{
    FastLED.show();
    enabled = true; // Enable the shift light functionality
}

void ShiftLight::disable()
{
    FastLED.clear(true);
    FastLED.show();
    enabled = false; // Disable the shift light functionality
}

/**
 * Updates the shift light strip based on current RPM.
 *
 * @param currentRpm The live engine RPM from your sensor/CAN bus.
 */
void ShiftLight::updateRPM(uint16_t currentRpm)
{
    if (enabled == false)
    {
        return; // Skip processing if the shift light is disabled
    }

    // 1. OVER-REV WARNING: Flash all LEDs Blue
    if (currentRpm >= _maxRpm)
    {
        // Toggle flash state every 50 milliseconds without blocking delay()
        if (millis() - lastFlashTime >= 50)
        {
            lastFlashTime = millis();
            flashState = !flashState;
        }

        if (flashState)
        {
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
        }
        else
        {
            FastLED.clear();
        }
        FastLED.show();
        return; // Skip standard progression logic
    }

    // 2. BELOW THRESHOLD: Turn all LEDs off
    if (currentRpm < _startRpm)
    {
        FastLED.clear(true);
        return;
    }

    // 3. PROGRESSIVE DISPLAY: Calculate how many LEDs should light up
    // Map current RPM between _startRpm and _shiftRpm to an LED count (1 to NUM_LEDS)
    uint8_t activeLeds = map(currentRpm, _startRpm, _shiftRpm, 1, NUM_LEDS);
    activeLeds = constrain(activeLeds, 1, NUM_LEDS);

    // Clear previous colors
    FastLED.clear();

    // 4. COLOR BINNING: Fill active LEDs based on position
    for (uint8_t i = 0; i < activeLeds; i++)
    {
        // Define color zone boundaries across the strip
        if (i < (NUM_LEDS * 0.30))
        {

            leds[i] = CRGB::Green;
        }
        else if (i < (NUM_LEDS * 0.60))
        {

            leds[i] = CRGB::Yellow;
        }
        else
        {

            leds[i] = CRGB::Red;
        }
    }

    FastLED.show();
}

void ShiftLight::setShiftLightRPMBins(uint16_t startRpm, uint16_t shiftRpm, uint16_t maxRpm)
{
    _startRpm = startRpm;
    _shiftRpm = shiftRpm;
    _maxRpm = maxRpm;
}