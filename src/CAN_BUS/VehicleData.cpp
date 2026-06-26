#include "VehicleData.h"

VehicleData::VehicleData()
    : rpm(0x5E8, 2, 1.0f, 50, CANDataType::U16),
      map(0x5E8, 0, 0.1f, 1.0f, CANDataType::S16),
      tps(0x5E8, 6, 0.1f, 1, CANDataType::S16),
      coolant(0x5E8, 4, 0.1f, 3, CANDataType::S16),
      pw1(0x5E9, 0, 0.001f, 1, CANDataType::U16),
      afr1(0x5E9, 4, 0.1f, 0.1f, CANDataType::U16),
      battery(0x5EA, 2, 0.1f, 0.1f, CANDataType::U16),
      fuelLevel(0x600, 0, 0.1f, 10.0f, CANDataType::U16),
      leftTurn(0x601, 0, 0.0f, 0.0f, CANDataType::BOOL),  //Byte 0
      rightTurn(0x601, 1, 1.0f, 0.0f, CANDataType::BOOL), //Byte 1
      highBeam(0x601, 2, 2.0f, 0.0f, CANDataType::BOOL),  //Byte 2
      headlights(0x601, 3, 3.0f, 0.0f, CANDataType::BOOL), //Byte 3
      speed(0x602, 0, 1.0f, 1.0f, CANDataType::U16), 
      button0(0x603, 0, 0.0f, 0.0f, CANDataType::BOOL), //Byte 0
      button1(0x603, 1, 1.0f, 0.0f, CANDataType::BOOL)  //Byte 1

{
}
