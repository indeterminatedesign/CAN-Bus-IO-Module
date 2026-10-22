
#pragma once

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define CAN_TX 4   // Physical pin for CAN TX
#define CAN_RX 5    // Physical pin for CAN RX

// Shift Light Configuration
#define NUM_LEDS 14
#ifndef LED_DATA_PIN
#define LED_DATA_PIN 18
#endif

#define PULSES_PER_MILE 4000  // This is the number of pulses per Mile expected from the vehicle speed sensor
#define SPEED_CALCULATION_INTERVAL_MS 500 // Interval for speed calculation in milliseconds
#define ODOMETER_SAVE_INTERVAL_MILES 1 // Persist odometer after each traveled mile
#define ODOMETER_SAVE_RETRY_INTERVAL_MS 60000 // Retry failed NVS writes no more than once per minute

#endif // CONFIGURATION_H

#define SPI_CS    	10 		   // SPI slave select
#define ADC_VREF    3300     // 3.3V Vref
#define ADC_CLK     1600000  // SPI clock 1.6MHz
#define SPLS        128      // samples

#define SHIFT_LED_GPIO 13       // Replace with your actual LED pin
#define SHIFT_RPM_THRESHOLD 6000 // RPM turning point for the shift light

// Interval:
#define TRANSMIT_RATE_MS 20  //Rate we transmit CAN BUS data in milliseconds
#define POLLING_RATE_MS 20 //Rate we poll digital and analog inputs in milliseconds
#define RECEIVE_RATE_MS 20 //Rate we receive CAN BUS data in milliseconds

//Mapping of physical ESP32 pins to digital GPIOs for CAN IO Board
#define DIGITAL0_GPIO 1
#define DIGITAL1_GPIO 2    
#define DIGITAL2_GPIO 6
#define DIGITAL3_GPIO 7  
#define DIGITAL4_GPIO 8   
#define DIGITAL5_GPIO 9    
#define DIGITAL6_GPIO 15  
#define DIGITAL7_GPIO 16    
#define DIGITAL8_GPIO 17       

//MAP GPIOs to a specific function.  For example, we tie the digital input 0 to the right turn signal.  Then when transmitting CAN Bus data we send Digital 0's state as the Right turn signal 
#define RIGHT_TURN_SIGNAL_GPIO DIGITAL0_GPIO
#define LEFT_TURN_SIGNAL_GPIO DIGITAL1_GPIO 
#define HIGH_BEAM_GPIO DIGITAL2_GPIO
#define HEADLIGHTS_GPIO DIGITAL3_GPIO
#define VSS_GPIO DIGITAL4_GPIO

//Analog Channels to function Mapping
#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1
#define ADC_CHANNEL_2 2
#define FUEL_LEVEL_CHANNEL 3

//Mapping of Button GPIOs
#define BUTTON_0_GPIO 14
#define BUTTON_1_GPIO 38

//Mapping of Output GPIOs
#define OUTPUT_0_GPIO 18
#define OUTPUT_1_GPIO 21

// Fuel level calculation constants (To be calibrated based on actual vehicle data)
#define FUEL_LEVEL_MAX 254
#define FUEL_LEVEL_RANGE 346
