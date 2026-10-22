#include "CANDataField.h"
#include <limits>

/**
 * @brief Decodes raw CAN data into the processed 'value' field.
 * Raw multi-byte values are extracted in big-endian order and scaled.
 * * @param data Pointer to the 8-byte CAN data array.
 */
void CANDataField::decode(const uint8_t *data)
{
    double raw = 0;
    switch (canDatatype)
    {
    case CANDataType::BOOL:
        raw = (data[offset] != 0) ? 1 : 0;
        break;
    case CANDataType::S16:
    {
        raw = static_cast<int16_t>(
            (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1]);
        break;
    }
    case CANDataType::U16:
    {
        raw = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
        break;
    }
    case CANDataType::U8:
    {
        raw = data[offset];
        break;
    }
    case CANDataType::U32:
    {
        raw = (static_cast<uint32_t>(data[offset]) << 24) |
              (static_cast<uint32_t>(data[offset + 1]) << 16) |
              (static_cast<uint32_t>(data[offset + 2]) << 8) |
              static_cast<uint32_t>(data[offset + 3]);
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

    case CANDataType::U32:
    {
        const double scaledValue = (value - sensorBias) / scale;
        const uint32_t raw = scaledValue <= 0
                                 ? 0
                                 : scaledValue >= std::numeric_limits<uint32_t>::max()
                                       ? std::numeric_limits<uint32_t>::max()
                                       : static_cast<uint32_t>(scaledValue);
        data[offset] = (raw >> 24) & 0xFF;
        data[offset + 1] = (raw >> 16) & 0xFF;
        data[offset + 2] = (raw >> 8) & 0xFF;
        data[offset + 3] = raw & 0xFF;
        break;
    }
    }
}
