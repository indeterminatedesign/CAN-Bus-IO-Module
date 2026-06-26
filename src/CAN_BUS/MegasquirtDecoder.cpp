#include "MegasquirtDecoder.h"

MegasquirtDecoder::MegasquirtDecoder(VehicleData &_signalsRef)
    : _vehicleSignalData(_signalsRef)
{
}

uint16_t MegasquirtDecoder::readU16(const uint8_t *d, uint8_t offset)
{
    return (d[offset] << 8) | d[offset + 1];
}

int16_t MegasquirtDecoder::readS16(const uint8_t *d, uint8_t offset)
{
    return (int16_t)((d[offset] << 8) | d[offset + 1]);
}

void MegasquirtDecoder::processFrame(const twai_message_t &msg)
{
    if (msg.extd)
        return;
    if (msg.data_length_code != 8)
        return;

    // Decode each of the different messages that megasquirt sends based on the CAN ID (The hex number in the case statement)
    switch (msg.identifier)
    {
    case 0x5E8:
        decodeGroup0(msg.data);
        break;
    case 0x5E9:
        decodeGroup1(msg.data);
        break;
    case 0x5EA:
        decodeGroup1(msg.data);
        break;
    case 0x5EB:
        decodeGroup2(msg.data);
        break;
    }
}

// 0x5E8
void MegasquirtDecoder::decodeGroup0(const uint8_t *d)
{
    _vehicleSignalData.map.value = readS16(d, 0) / 10.0f;

    _vehicleSignalData.rpm.value = readU16(d, 2);

    _vehicleSignalData.coolant.value = readS16(d, 4) / 10.0f;

    _vehicleSignalData.tps.value = readS16(d, 6) / 10.0f;
}
// 0x5E9
void MegasquirtDecoder::decodeGroup1(const uint8_t *d)
{
    _vehicleSignalData.pw1.value = readU16(d, 0) / 1000.0f;

    _vehicleSignalData.iat.value = readS16(d, 4) / 10.0f;
}
// 0x5EA
void MegasquirtDecoder::decodeGroup2(const uint8_t *d)
{
    _vehicleSignalData.afr1.value = readU16(d, 1) / 10.0f;
}
// 0x5EB
void MegasquirtDecoder::decodeGroup3(const uint8_t *d)
{
    _vehicleSignalData.battery.value = readU16(d, 2) / 10.0f;
}
