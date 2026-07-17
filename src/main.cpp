#include <Arduino.h>
#include "CAN_Bus/CANBus.h"
#include "CAN_Bus/CANIODataEncoder.h"
#include "configuration.h"
#include "CAN_BUS/CANDataField.h"
#include "CAN_BUS/VehicleData.h"
#include <SPI.h>
#include "MCP_ADC.h"
#include <freertos/FreeRTOS.h>
#include "CAN_BUS/MegasquirtDecoder.h"

// Buffers used exclusively by Core 1 (Polling & Diagnostics)
VehicleData vehicleOutputData;
VehicleData vehicleInputDataReadBuffer;

// Buffers used exclusively by Core 0 (CAN Transmit / Decode)
VehicleData vehicleDataOutputBuffer;
VehicleData vehicleInputData;

CANBus canBus;
CANIODataEncoder ioEncoder;
MegasquirtDecoder msDecoder(vehicleInputData);
MCP3204 adc;

SemaphoreHandle_t vehicleDataMutex = NULL;
static portMUX_TYPE vssMux = portMUX_INITIALIZER_UNLOCKED;

// Function Prototypes
void updateInputs();
void updateOutputs();
uint16_t calculateSpeedFromVss();
float emaFilter(float newValue, float prevEma, float alpha);
float measureFuel();
void printDiagnostics();

volatile uint32_t vssPulseCount = 0;
unsigned long lastPollMillis = 0;
unsigned long lastDiagnosticsMillis = 0;

void IRAM_ATTR onVssPulse()
{
  portENTER_CRITICAL_ISR(&vssMux);
  vssPulseCount += 1;
  portEXIT_CRITICAL_ISR(&vssMux);
}

// ----------------- CAN TRANSMIT/RECEIVE TASK (CORE 0) -----------------
void canTask(void *param)
{
  TickType_t lastTransmitTick = xTaskGetTickCount();

  // Convert your TRANSMIT_RATE_MS config into FreeRTOS ticks (e.g., 20ms or 50ms)
  const TickType_t transmitIntervalTicks = pdMS_TO_TICKS(TRANSMIT_RATE_MS);

  while (true)
  {
    twai_message_t msg;
    bool hasIncomingData = false;

    // 1. Drain incoming frames if another node speaks to this module
    while (canBus.receiveMessage(msg))
    {
      hasIncomingData = true;

      // If you are using your new ID-based self-decoding VehicleData here,
      // you would change this block to check your ID map/registry:
      if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EA)
      {
        msDecoder.processFrame(msg);
      }
    }

    // 2. Check if it's time to transmit our data over the CAN bus
    TickType_t currentTick = xTaskGetTickCount();
    bool timeToTransmit = (currentTick - lastTransmitTick >= transmitIntervalTicks);

    // 3. Keep the cross-core lock fast and ultra-short
    if (vehicleDataMutex != NULL)
    {
      if (xSemaphoreTake(vehicleDataMutex, pdMS_TO_TICKS(1)) == pdTRUE)
      {
        // Swap Incoming data to Core 1 if needed
        if (hasIncomingData)
        {
          vehicleInputDataReadBuffer = vehicleInputData;
        }

        // Swap Outgoing data from Core 1 down to Core 0
        if (timeToTransmit)
        {
          vehicleDataOutputBuffer = vehicleOutputData;
        }

        xSemaphoreGive(vehicleDataMutex);
      }
    }

    // 4. Send the frames out at your exact configured regular interval
    if (timeToTransmit)
    {
      ioEncoder.sendFrame(vehicleDataOutputBuffer, canBus);
      lastTransmitTick = currentTick; // Reset transmit interval clock
    }

    // Yield for 1ms to prevent core lockup while maximizing queue response
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup()
{
  Serial.begin(512000);
  Serial.println("CAN IO Board Starting...");

  canBus.begin();

  vehicleDataMutex = xSemaphoreCreateMutex();
  if (vehicleDataMutex == NULL)
  {
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

  // Configure GPIO pins for digital outputs
  pinMode(SHIFT_LED_GPIO, OUTPUT);
  digitalWrite(SHIFT_LED_GPIO, LOW);

  // Initialize MCP3204 ADC
  SPI.begin();
  adc.begin(SPI_CS);

  // Setup interrupt for VSS pulse counting
  pinMode(VSS_GPIO, INPUT);
  attachInterrupt(VSS_GPIO, onVssPulse, RISING);

  // Launch the core CAN management task on Core 0
  xTaskCreatePinnedToCore(
      canTask, "CAN Task", 4096, NULL, 1, NULL, 0);

  Serial.println("Setup complete");
}

// ----------------- SENSOR ACQUISITION & CONTROL LOOP (CORE 1) -----------------
void loop()
{
  unsigned long now = millis();

  // Poll your raw analog and digital inputs, and update outputs at a fixed rate
  if (now - lastPollMillis >= POLLING_RATE_MS)
  {
    lastPollMillis = now;
    updateInputs();  // Read local physical pins -> Write to vehicleOutputData
    updateOutputs(); // Read vehicleInputDataReadBuffer -> Control physical pins
  }

  // Output diagnostics to Serial at a fixed rate (200ms)
  if (now - lastDiagnosticsMillis >= 200)
  {
    lastDiagnosticsMillis = now;
    printDiagnostics();
  }

  vTaskDelay(pdMS_TO_TICKS(1));
}

void updateInputs()
{
  // 1. Gather all inputs onto Core 1 stack safely without locking anything
  bool right = digitalRead(RIGHT_TURN_SIGNAL_GPIO);
  bool left = digitalRead(LEFT_TURN_SIGNAL_GPIO);
  bool high = digitalRead(HIGH_BEAM_GPIO);
  bool head = digitalRead(HEADLIGHTS_GPIO);
  uint16_t spd = calculateSpeedFromVss();
  float fuel = measureFuel();
  bool b0 = digitalRead(BUTTON_0_GPIO);
  bool b1 = digitalRead(BUTTON_1_GPIO);

  // 2. Perform a split-second structural copy to stage it for Core 0
  if (xSemaphoreTake(vehicleDataMutex, portMAX_DELAY) == pdTRUE)
  {
    vehicleOutputData.rightTurn.value = right;
    vehicleOutputData.leftTurn.value = left;
    vehicleOutputData.highBeam.value = high;
    vehicleOutputData.headlights.value = head;
    vehicleOutputData.speed.value = spd;
    vehicleOutputData.fuelLevel.value = fuel;
    vehicleOutputData.button0.value = b0;
    vehicleOutputData.button1.value = b1;
    xSemaphoreGive(vehicleDataMutex);
  }
}

void updateOutputs()
{
  // Local stack copy of our incoming data to avoid holding the lock
  VehicleData inputSnapshot;

  // 1. Lock and grab a fast snapshot of incoming data from Megasquirt
  if (vehicleDataMutex != NULL)
  {
    if (xSemaphoreTake(vehicleDataMutex, pdMS_TO_TICKS(2)) == pdTRUE)
    {
      inputSnapshot = vehicleInputDataReadBuffer;
      xSemaphoreGive(vehicleDataMutex);
    }
    else
    {
      // If the mutex is momentarily busy, skip this tick to keep loop responsive
      return;
    }
  }

  // 2. Execute local control logic based on Megasquirt data
  // (Assumes 'rpm' is defined in your VehicleData structure)
  if (inputSnapshot.rpm.value >= SHIFT_RPM_THRESHOLD)
  {
    digitalWrite(SHIFT_LED_GPIO, HIGH);
  }
  else
  {
    digitalWrite(SHIFT_LED_GPIO, LOW);
  }
}

uint16_t calculateSpeedFromVss()
{
  static uint32_t lastSpeedCalculationMillis = 0;
  uint16_t speed = 0;
  uint32_t now = millis();
  uint32_t deltaTime = now - lastSpeedCalculationMillis;

  if (deltaTime > 0)
  {
    portENTER_CRITICAL(&vssMux);
    uint32_t currentCount = vssPulseCount;
    vssPulseCount = 0;
    portEXIT_CRITICAL(&vssMux);

    // Calculate the speed in mph
    float pulsesPerMillisecond = (float)currentCount / (float)deltaTime;
    float pulsesPerHour = pulsesPerMillisecond * 3600000.0;

    speed = (uint16_t)(pulsesPerHour / (float)PULSES_PER_MILE);
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
  return constrain((filtered / 4095.0f) * 100.0f, 0.0f, 100.0f);
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