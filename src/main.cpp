#include <Arduino.h>
#include "CAN_Bus/CANBus.h"
#include "CAN_Bus/CANIODataEncoder.h"
#include "configuration.h"
#include "CAN_BUS/CANDataField.h"
#include "CAN_BUS/VehicleData.h"
#include <SPI.h>
#include "MCP_ADC.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "CAN_BUS/MegasquirtDecoder.h"

VehicleData vehicleOutputData;  //Final data to be sent over CAN bus
VehicleData vehicleDataOutputBuffer; // For double buffering output data
VehicleData vehicleInputData;   // Data received from CAN bus
VehicleData vehicleInputDataReadBuffer;   // Data received from CAN bus
CANBus canBus;
CANIODataEncoder ioEncoder;
MegasquirtDecoder msDecoder(vehicleInputData);
MCP3204 adc;
SemaphoreHandle_t vehicleDataMutex = NULL;

void updateInputs();
void transmitVehicleData();
uint16_t calculateSpeedFromVss();
float emaFilter(float newValue, float prevEma, float alpha);
float measureFuel();
void printDiagnostics();

volatile uint32_t vssPulseCount = 0;
unsigned long lastPollMillis = 0;
unsigned long lastTransmitMillis = 0;
unsigned long lastReceiveMillis = 0;
unsigned long lastDiagnosticsMillis = 0;

void IRAM_ATTR onVssPulse()
{
  vssPulseCount +=1;
}

// ----------------- CAN Task -----------------
void canRecieveTask(void *param)
{
  while (true)
  {
    twai_message_t msg;
    if (canBus.receiveMessage(msg))
    {
      // Route to correct decoder
      if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EA)
        msDecoder.processFrame(msg);

      // Atomic update for UI
      vehicleInputDataReadBuffer = vehicleInputData;
    }

    vTaskDelay(pdMS_TO_TICKS(RECEIVE_RATE_MS)); // ms delay
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("CAN IO Board Starting...") ;
  canBus.begin();

  vehicleDataMutex = xSemaphoreCreateMutex();
  if (vehicleDataMutex == NULL)
  {
    Serial.println("❌ Failed to create vehicle data mutex");
    while (1)
      ;
  }

  // Configure GPIO pins for digital inputs
  pinMode(DIGITAL0_GPIO, INPUT);
  pinMode(DIGITAL1_GPIO, INPUT);
  pinMode(DIGITAL2_GPIO, INPUT);
  pinMode(DIGITAL3_GPIO, INPUT);
  pinMode(DIGITAL4_GPIO, INPUT);
  pinMode(DIGITAL5_GPIO, INPUT);
  pinMode(DIGITAL6_GPIO, INPUT);
  pinMode(DIGITAL7_GPIO, INPUT);
  pinMode(DIGITAL8_GPIO, INPUT);
  
  pinMode(BUTTON_0_GPIO, INPUT);
  pinMode(BUTTON_1_GPIO, INPUT);

  // Initialize MCP3204 ADC
  SPI.begin();
  adc.begin(SPI_CS);
  
  //Setup interrupt for VSS pulse counting
  pinMode(VSS_GPIO, INPUT);
  attachInterrupt(VSS_GPIO, onVssPulse, RISING);

    // Create CAN task pinned to core 0
  xTaskCreatePinnedToCore(
      canRecieveTask, // Task function
      "CAN Task",     // Name
      4096,           // Stack size (bytes)
      NULL,           // Parameter
      1,              // Priority
      NULL,           // Task handle
      0               // Core ID
  );

  Serial.println("Setup complete");
}


void loop()
{
  unsigned long now = millis();

  // Poll inputs at a fixed rate
  if (now - lastPollMillis >= POLLING_RATE_MS)
  {
    lastPollMillis = now;
    updateInputs();
  }

  //Transmit vehicle data at a fixed rate via CAN bus
  if (now - lastTransmitMillis >= TRANSMIT_RATE_MS)
  {
    lastTransmitMillis = now;
    transmitVehicleData();
  }

  if(now - lastReceiveMillis >= RECEIVE_RATE_MS)
  {
    lastReceiveMillis = now;
  }

  //Output diagnostics to Serial at a fixed rate
  if (now - lastDiagnosticsMillis >= 200)  // Print diagnostics every .2 seconds
  {
    lastDiagnosticsMillis = now;
    printDiagnostics();
  }
}

void updateInputs()
{
  if (xSemaphoreTake(vehicleDataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
  {
    vehicleDataOutputBuffer.rightTurn.boolValue = digitalRead(RIGHT_TURN_SIGNAL_GPIO);
    vehicleDataOutputBuffer.leftTurn.boolValue = digitalRead(LEFT_TURN_SIGNAL_GPIO);
    vehicleDataOutputBuffer.highBeam.boolValue = digitalRead(HIGH_BEAM_GPIO);
    vehicleDataOutputBuffer.headlights.boolValue = digitalRead(HEADLIGHTS_GPIO);
    vehicleDataOutputBuffer.speed.value = calculateSpeedFromVss();
    vehicleDataOutputBuffer.fuelLevel.value = measureFuel();
    vehicleDataOutputBuffer.button0.boolValue = digitalRead(BUTTON_0_GPIO);
    vehicleDataOutputBuffer.button1.boolValue = digitalRead(BUTTON_1_GPIO);
    xSemaphoreGive(vehicleDataMutex);
  }
}

void transmitVehicleData()
{
  if (xSemaphoreTake(vehicleDataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
  {
    vehicleOutputData = vehicleDataOutputBuffer;
    xSemaphoreGive(vehicleDataMutex);
  }
  ioEncoder.sendFrame(vehicleOutputData, canBus);
}

uint16_t calculateSpeedFromVss()
{
  static uint32_t lastSpeedCalculationMillis = 0;
  uint16_t speed = 0;
  uint32_t now = millis();
  uint32_t deltaTime = now - lastSpeedCalculationMillis;

  if (deltaTime > 0)
  {
    // --- START CRITICAL SECTION ---
    // This prevents the interrupt from changing the value while we read/clear it
    portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&myMutex);
    
    uint32_t currentCount = vssPulseCount;
    vssPulseCount = 0; // Reset it safely
    
    portEXIT_CRITICAL(&myMutex);
    // --- END CRITICAL SECTION ---

    speed = (currentCount * 1000UL) / deltaTime;
  }

  lastSpeedCalculationMillis = now;
  return speed;
}

float measureFuel()
{
  static float lastFuelLevel = 0.0f;
  uint16_t rawAdc = adc.read(FUEL_LEVEL_CHANNEL);
  float filtered = emaFilter((float)rawAdc, lastFuelLevel, 0.01f);
  lastFuelLevel = filtered;

  float percent = (filtered / 4095.0f) * 100.0f;
  return constrain(percent, 0.0f, 100.0f);
}

float emaFilter(float newValue, float prevEma, float alpha)
{
  return (alpha * newValue) + ((1.0f - alpha) * prevEma);
}

void printDiagnostics()
{
  Serial.println("\n========== INPUT DIAGNOSTICS ==========");
  
  // Digital GPIO Inputs (0-8)
  Serial.println("\nDIGITAL INPUTS:");
  Serial.print("  DIGITAL0 (GPIO 1):  ");
  Serial.println(digitalRead(DIGITAL0_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL1 (GPIO 2):  ");
  Serial.println(digitalRead(DIGITAL1_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL2 (GPIO 6):  ");
  Serial.println(digitalRead(DIGITAL2_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL3 (GPIO 7):  ");
  Serial.println(digitalRead(DIGITAL3_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL4 (GPIO 8):  ");
  Serial.println(digitalRead(DIGITAL4_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL5 (GPIO 9):  ");
  Serial.println(digitalRead(DIGITAL5_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL6 (GPIO 15): ");
  Serial.println(digitalRead(DIGITAL6_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL7 (GPIO 16): ");
  Serial.println(digitalRead(DIGITAL7_GPIO) ? "HIGH" : "LOW");
  Serial.print("  DIGITAL8 (GPIO 17): ");
  Serial.println(digitalRead(DIGITAL8_GPIO) ? "HIGH" : "LOW");
  
  // Button Inputs
  Serial.println("\nBUTTON INPUTS:");
  Serial.print("  BUTTON_0 (GPIO 14): ");
  Serial.println(digitalRead(BUTTON_0_GPIO) ? "RELEASED" : "PRESSED");
  Serial.print("  BUTTON_1 (GPIO 38): ");
  Serial.println(digitalRead(BUTTON_1_GPIO) ? "RELEASED" : "PRESSED");
  
  // ADC Channel Readings from MCP3204
  Serial.println("\nADC INPUTS (MCP3204):");
  
  // Read each channel with small delay between reads
  uint16_t adc0 = adc.read(0);
  delayMicroseconds(100);
  uint16_t adc1 = adc.read(1);
  delayMicroseconds(100);
  uint16_t adc2 = adc.read(2);
  delayMicroseconds(100);
  uint16_t adc3 = adc.read(3);
  
  Serial.print("  ADC_CHANNEL_0: ");
  Serial.print(adc0);
  Serial.print(" (0x");
  Serial.print(adc0, HEX);
  Serial.println(")");
  
  Serial.print("  ADC_CHANNEL_1: ");
  Serial.print(adc1);
  Serial.print(" (0x");
  Serial.print(adc1, HEX);
  Serial.println(")");
  
  Serial.print("  ADC_CHANNEL_2: ");
  Serial.print(adc2);
  Serial.print(" (0x");
  Serial.print(adc2, HEX);
  Serial.println(")");
  
  Serial.print("  ADC_CHANNEL_3 (FUEL): ");
  Serial.print(adc3);
  Serial.print(" (0x");
  Serial.print(adc3, HEX);
  Serial.println(")");
  
  Serial.println("======================================\n");
}