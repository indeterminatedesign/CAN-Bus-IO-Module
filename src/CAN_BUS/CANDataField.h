#pragma once
#include <Arduino.h>

/**
 * @brief Supported data types for CAN message extraction.
 */
enum class CANDataType
{
    BOOL, ///< Boolean value (0 or 1)
    U16,  ///< Unsigned 16-bit integer
    S16,  ///< Signed 16-bit integer
    Float ///< Floating point value
};

/**
 * @brief Represents a single data field within a CAN message.
 * * This class handles the metadata, storage, and conversion logic for
 * translating raw CAN bus bytes into meaningful physical values.
 */
class CANDataField
{
public:
    CANDataType type;        ///< The underlying data format in the CAN frame
    float value = 0.0f;      ///< The processed physical value (scaled)
    bool boolValue = false;   ///< The boolean value for CANDataType::BOOL fields
    uint32_t lastUpdate = 0; ///< Timestamp (ms) of the last successful decode

    // CAN mapping
    uint32_t canId;          ///< The specific CAN Arbitration ID for this field
    uint8_t offset;          ///< Byte index in the 8-byte CAN payload where data starts
    float scale;             ///< Multiplier to convert raw bits to physical units (e.g., 0.1 for 100mV -> 10V)
    float uiUpdateTolerance; ///< Minimum change required in 'value' to trigger a UI refresh

    /**
     * @brief Construct a new CANDataField object.
     * * @param id The CAN Message ID to listen for.
     * @param off The starting byte position (0-7) within the CAN frame.
     * @param scl The scaling factor applied to the raw value.
     * @param tolerance The delta threshold for UI updates.
     * @param t The data type (U16, S16, or Float).
     */
    CANDataField(uint32_t id = 0, uint8_t off = 0, float scl = 1.0f, float tolerance = 1.0f, CANDataType t = CANDataType::U16)
        : canId(id),
          offset(off),
          scale(scl),
          uiUpdateTolerance(tolerance),
          type(t)
    {
    }

    /**
     * @brief Decodes raw CAN data into the processed 'value' field.
     * * This method extracts two bytes starting at 'offset' (Big-Endian),
     * casts them based on 'type', applies the 'scale', and updates the timestamp.
     * * @note When using CANDataType::Float, ensure the raw data actually fits
     * into the 16 bits extracted, otherwise, pointer aliasing may cause undefined behavior.
     * * @param data Pointer to the 8-byte CAN data array.
     */
    void decode(const uint8_t *data)
    {
        // Extracts 16 bits (Big-Endian)
        uint16_t raw = (data[offset] << 8) | data[offset + 1];

        switch (type)
        {
        case CANDataType::BOOL:
            boolValue = (raw != 0);
            value = boolValue ? 1.0f : 0.0f;
            break;
        case CANDataType::S16:
            value = ((int16_t)raw) * scale;
            break;
        case CANDataType::U16:
            value = raw * scale;
            break;
        case CANDataType::Float:
            // Reinterprets the 16-bit raw data as a float bit-pattern
            value = *((float *)&raw) * scale;
            break;
        }
        lastUpdate = millis();
    }
    
    void encode(uint8_t *data)
    {
        uint16_t raw = 0;
        switch (type)
        {
        case CANDataType::BOOL:
            raw = boolValue ? 1 : 0;
            break;
        case CANDataType::S16:
            raw = (int16_t)(value / scale);
            break;
        case CANDataType::U16:
            raw = (uint16_t)(value / scale);
            break;
        case CANDataType::Float:
            // Reinterprets the float value as a 16-bit bit-pattern
            raw = *((uint16_t *)&value);
            break;
        }
        data[offset] = (raw >> 8) & 0xFF;     // High byte
        data[offset + 1] = raw & 0xFF;         // Low byte
    }
};