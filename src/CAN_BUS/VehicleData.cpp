#include "VehicleData.h"
// Id, offset, scale, update tolerance
VehicleData::VehicleData()
    : rpm(0x5E8, 2, 1.00f, 0.0f, 0.0f, CANDataType::U16),    // Offset 2, scale 1.0
      map(0x5E8, 0, 0.1f, 0.0f, 0.0f, CANDataType::S16),     // Offset 0, scale 0.1
      tps(0x5E8, 6, 0.1f, 5.0f, 0.0f, CANDataType::S16),     // Offset 6, scale 0.1
      coolant(0x5E8, 4, 0.1f, 0.0f, 0.0f, CANDataType::S16), // Offset 4, scale 0.1
      pw1(0x5E9, 0, 0.001f, 0.0f, 0.0f, CANDataType::U16),   // Changed to 0x5E9, Offset 0, scale 0.001
      iat(0x5E9, 4, 0.1f, 0.0f, 0.0f, CANDataType::S16),     // Corrected scale parameters
      afr1(0x5EA, 1, 0.1f, 0.0f, 0.0f, CANDataType::U8),
      battery(0x5EB, 0, 0.1f, 0.0f, 0.0f, CANDataType::U16), // Offset 0, scale 0.1
      fuelLevel(0x600, 0, 1.0f, 0.0f, 0.0f, CANDataType::U16),
      leftTurn(0x601, 0, 1.0f, 0.0f, 0.0f, CANDataType::BOOL),
      rightTurn(0x601, 1, 1.0f, 0.0f, 0.0f, CANDataType::BOOL),
      highBeam(0x601, 2, 1.0f, 0.0f, 0.0f, CANDataType::BOOL),
      headlights(0x601, 3, 1.0f, 0.0f, 0.0f, CANDataType::BOOL),
      speed(0x602, 0, 1.0f, 0.0f, 0.0f, CANDataType::U16),
      button0(0x603, 0, 1.0f, 0.0f, 0.0f, CANDataType::BOOL),
      button1(0x603, 1, 1.0f, 0.0f, 0.0f, CANDataType::BOOL)
{
}