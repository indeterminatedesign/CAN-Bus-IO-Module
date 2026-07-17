#include "CANDataField.h"

/**
 * @brief Decodes raw CAN data into the processed 'value' field.
 * * This method extracts two bytes starting at 'offset' (Big-Endian),
 * casts them based on 'type', applies the 'scale', and updates the timestamp.
 * * @note When using CANDataType::Float, ensure the raw data actually fits
 * into the 16 bits extracted, otherwise, pointer aliasing may cause undefined behavior.
 * * @param data Pointer to the 8-byte CAN data array.
 */
void CANDataField::decode(const uint8_t *data)
{
    float raw;
    switch (canDatatype)
    {
    case CANDataType::BOOL:
        value = (data[offset] != 0) ? 1 : 0;
        break;
    case CANDataType::S16:
    {
        raw = (int16_t)((data[offset] << 8) | data[offset + 1]);
        value = ((int16_t)raw) * scale;
        break;
    }
    case CANDataType::U16:
    {
        raw = (data[offset] << 8) | data[offset + 1];
        break;
    }
    case CANDataType::U8:
    {
        raw = data[offset];
        break;
    }
    }
    value = (raw * scale) + sensorBias;

    lastUpdate = millis();
}

void CANDataField::encode(uint8_t *data)
{
    switch (canDatatype)
    {
    case CANDataType::BOOL:
        // Boolean values generally do not use scale/bias
        data[offset] = (value != 0) ? 1 : 0;
        break;
        
    case CANDataType::S16:
    {
        // Subtract bias first, then divide by scale
        int16_t raw = (int16_t)((value - sensorBias) / scale);
        data[offset]     = (raw >> 8) & 0xFF; // High byte
        data[offset + 1] = raw & 0xFF;        // Low byte
        break;
    }
    
    case CANDataType::U16:
    {
        // Subtract bias first, then divide by scale
        uint16_t raw = (uint16_t)((value - sensorBias) / scale);
        data[offset]     = (raw >> 8) & 0xFF; // High byte
        data[offset + 1] = raw & 0xFF;        // Low byte
        break;
    }
    
    case CANDataType::U8:
    {
        // Subtract bias first, then divide by scale
        uint8_t raw = (uint8_t)((value - sensorBias) / scale);
        data[offset] = raw;
        break;
    }
    }
}
