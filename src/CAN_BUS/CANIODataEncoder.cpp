#include "CANIODataEncoder.h"

CANIODataEncoder::CANIODataEncoder()
{
}

void CANIODataEncoder::sendFrame(VehicleData& data, CANBus& _canBus)
{
    uint8_t frame[8] = {0};

    // Fuel level
    memset(frame, 0, sizeof(frame));
    data.fuelLevel.encode(frame);
    _canBus.sendMessage(0x600, frame, sizeof(frame));

    // Turn signals and lights
    memset(frame, 0, sizeof(frame));
    data.leftTurn.encode(frame);
    data.rightTurn.encode(frame);
    data.highBeam.encode(frame);
    data.headlights.encode(frame);
    _canBus.sendMessage(0x601, frame, sizeof(frame));

    // Speed
    memset(frame, 0, sizeof(frame));
    data.speed.encode(frame);
    _canBus.sendMessage(0x602, frame, sizeof(frame));

    // Buttons
    memset(frame, 0, sizeof(frame));
    data.button0.encode(frame);
    data.button1.encode(frame);
    _canBus.sendMessage(0x603, frame, sizeof(frame));
}