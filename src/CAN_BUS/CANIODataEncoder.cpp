#include "CANIODataEncoder.h"

CANIODataEncoder::CANIODataEncoder()
{
}

void CANIODataEncoder::sendFrame(VehicleData& data, CANBus& _canBus) {
    // Isolate arrays completely to prevent stack/pointer leaks
    uint8_t frame600[8] = {0};
    uint8_t frame601[8] = {0};
    uint8_t frame602[8] = {0};
    uint8_t frame603[8] = {0};

    // 0x600 - Fuel
    data.fuelLevel.encode(frame600);
    _canBus.sendMessage(0x600, frame600, 8);
    delayMicroseconds(500); 

    // 0x601 - Signals/Lights
    data.leftTurn.encode(frame601);
    data.rightTurn.encode(frame601);
    data.highBeam.encode(frame601);
    data.headlights.encode(frame601);
    _canBus.sendMessage(0x601, frame601, 8);
    delayMicroseconds(500);

    // 0x602 - Speed
    data.speed.encode(frame602);
    _canBus.sendMessage(0x602, frame602, 8);
    delayMicroseconds(500);

    // 0x603 - Buttons
    data.button0.encode(frame603);
    data.button1.encode(frame603);
    _canBus.sendMessage(0x603, frame603, 8);
}
