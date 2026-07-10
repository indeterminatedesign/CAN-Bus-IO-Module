#include "CANBus.h"
void CANBus::begin()
{
    // --- CAN configuration ---
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX,
        (gpio_num_t)CAN_RX,
        TWAI_MODE_NORMAL);

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        Serial.println("❌ CAN driver install failed");
        while (1)
            ;
    }

    // Start CAN
    if (twai_start() != ESP_OK)
    {
        Serial.println("❌ CAN start failed");
        while (1)
            ;
    }
}
void CANBus::sendMessage(uint32_t id, uint8_t *data, uint8_t len) {
    if (len > 8) {
        Serial.printf("❌ TX Aborted: Data length %d exceeds 8 bytes.\n", len);
        return;
    }

    twai_message_t tx_msg = {};
    tx_msg.identifier = id;
    tx_msg.data_length_code = len;
    tx_msg.flags = (id > 0x7FF) ? TWAI_MSG_FLAG_EXTD : TWAI_MSG_FLAG_NONE;
    memcpy(tx_msg.data, data, len);

    esp_err_t result = twai_transmit(&tx_msg, pdMS_TO_TICKS(15));

    if (result == ESP_OK) {
        Serial.printf("✅ TX Success: ID=0x%03X, Len=%d\n", id, len);
    } 
    else {
        Serial.printf("❌ TX Failed for ID=0x%03X: ", id);
        
        switch (result) {
            case ESP_ERR_TIMEOUT:
                Serial.print("Timeout! ");
                break;
            case ESP_ERR_INVALID_STATE:
                Serial.print("Invalid Driver State! ");
                break;
            case ESP_ERR_INVALID_ARG:
                Serial.print("Invalid Argument! ");
                break;
            case ESP_FAIL:
                Serial.print("TX Queue Full! ");
                break;
            default:
                Serial.printf("Error Code 0x%X ", result);
                break;
        }

        // --- Fetch and Print Hardware Error Counters ---
        twai_status_info_t status_info;
        if (twai_get_status_info(&status_info) == ESP_OK) {
            Serial.printf("[HW Counters -> TX Errors: %d | RX Errors: %d | State: ", 
                          status_info.tx_error_counter, 
                          status_info.rx_error_counter);
            
            switch (status_info.state) {
                case TWAI_STATE_STOPPED:      Serial.println("STOPPED]"); break;
                case TWAI_STATE_RUNNING:      Serial.println("RUNNING]"); break;
                case TWAI_STATE_BUS_OFF:      Serial.println("BUS-OFF]"); break;
                default:                      Serial.println("UNKNOWN]"); break;
            }

            // Self-healing: If the counters forced the hardware into BUS-OFF, reset it
            if (status_info.state == TWAI_STATE_BUS_OFF) {
                Serial.println("⚠️ Hardware is BUS-OFF. Triggering auto-recovery...");
                if (twai_initiate_recovery() == ESP_OK) {
                    delay(100); 
                    twai_start();
                }
            }
        } else {
            Serial.println("[Failed to read HW counters]");
        }
    }
}


bool CANBus::receiveMessage(twai_message_t &rx_msg)
{
    if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK)
    {
        Serial.printf(
            "RX: ID=0x%03X DLC=%d DATA:",
            rx_msg.identifier,
            rx_msg.data_length_code);

        for (int i = 0; i < rx_msg.data_length_code; i++)
        {
            Serial.printf(" %02X", rx_msg.data[i]);
        }
        Serial.println();
        return true;
    }
    else
    {
        // No message received within timeout
        Serial.println("No Message Recieved");
        return false;
    }
}
