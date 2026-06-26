#pragma once
#include "VehicleData.h"
#include "driver/twai.h"
#include "CANBus.h"

class CANIODataEncoder
{
public:
    CANIODataEncoder();
    void sendFrame(VehicleData& data, CANBus& canBus);
private:
};