#pragma once
#include <Arduino.h>
#include "CANDataField.h"

class VehicleData
{
public:
    // Megasquirt fields
    CANDataField rpm;
    CANDataField map;
    CANDataField tps;
    CANDataField coolant;
    CANDataField iat;
    CANDataField afr1;
    CANDataField pw1;
    CANDataField battery;

    // CAN IO Module fields
    CANDataField highBeam;
    CANDataField headlights;
    CANDataField fuelLevel;
    CANDataField leftTurn;
    CANDataField rightTurn;
    CANDataField speed;
    CANDataField button0;
    CANDataField button1;

    VehicleData();

    void swapBuffers(); // optional if you implement double-buffering
};
