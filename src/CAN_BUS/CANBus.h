#pragma once
#include <Arduino.h>
#include "driver/twai.h"
#include "configuration.h"

class CANBus
{   
public:
    void begin();
    void sendMessage(uint32_t id, uint8_t* data, uint8_t len);
    bool receiveMessage(twai_message_t &rx_msg); 
};