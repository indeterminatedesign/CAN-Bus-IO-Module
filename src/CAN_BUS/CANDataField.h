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
    U8    // Unsigned byte
};

/**
 * @brief Represents a single data field within a CAN message.
 * * This class handles the metadata, storage, and conversion logic for
 * translating raw CAN bus bytes into meaningful physical values.
 */
class CANDataField
{
public:
    CANDataType canDatatype; ///< The underlying data format in the CAN frame
    float_t value = 0;       ///< The processed physical value (scaled)
    uint32_t lastUpdate = 0; ///< Timestamp (ms) of the last successful decode

    // CAN mapping
    uint32_t canId;          ///< The specific CAN Arbitration ID for this field
    uint8_t offset;          ///< Byte index in the 8-byte CAN payload where data starts
    float scale;             ///< Multiplier to convert raw bits to physical units (e.g., 0.1 for 100mV -> 10V)
    float uiUpdateTolerance; ///< Minimum change required in 'value' to trigger a UI refresh
    float sensorBias;

    /**
     * @brief Construct a new CANDataField object.
     * * @param id The CAN Message ID to listen for.
     * @param off The starting byte position (0-7) within the CAN frame.
     * @param scl The scaling factor applied to the raw value.
     * @param tolerance The delta threshold for UI updates.
     * @param t The data type (U16, S16, or Float).
     */
    CANDataField(uint32_t id = 0, uint8_t off = 0, float scl = 1.0f, float tolerance = 1.0f,float bias = 0.0f, CANDataType t = CANDataType::U16)
        : canId(id),
          offset(off),
          scale(scl),
          uiUpdateTolerance(tolerance),
          sensorBias(bias),
          canDatatype(t)
    {
    };
    void decode(const uint8_t *data);
    void encode(uint8_t *data);
};