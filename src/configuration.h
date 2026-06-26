
#define CAN_TX 4   // Physical pin for CAN TX
#define CAN_RX 5    // Physical pin for CAN RX

#define SPI_CS    	10 		   // SPI slave select
#define ADC_VREF    3300     // 3.3V Vref
#define ADC_CLK     1600000  // SPI clock 1.6MHz
#define SPLS        128      // samples

// Interval:
#define TRANSMIT_RATE_MS 50  //Rate we transmit CAN BUS data
#define POLLING_RATE_MS 50 //Rate we poll digital and analog inputs
#define RECEIVE_RATE_MS 50 //Rate we receive CAN BUS data

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

