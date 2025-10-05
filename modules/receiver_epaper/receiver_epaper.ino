
/*
 * Board: ESP32 Arduino -> Vision Master E290
 */

#include <heltec-eink-modules.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "../definitions.h"

// =================================================================
// ==                      CONFIGURATION                          ==
// =================================================================

// -- LoRa & Radio Settings --
#define RF_FREQUENCY                  433000000 // Hz
#define TX_OUTPUT_POWER               14        // dBm
#define LORA_BANDWIDTH                0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_CODINGRATE               1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_SYMBOL_TIMEOUT           0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON    false
#define LORA_IQ_INVERSION_ON          false

// -- Application Settings --
#define BUTTON_PIN                    21
#define DEBOUNCE_DELAY_MS             50
#define BUFFER_SIZE                   255 

// =================================================================
// ==                      GLOBAL VARIABLES                       ==
// =================================================================

// -- Display & State Machine --
EInkDisplay_VisionMasterE290 display;

typedef enum {
  STATE_DAILY,
  STATE_WEEKLY,
  STATE_MONTHLY
} DisplayState_t;

RTC_DATA_ATTR DisplayState_t currentDisplayState = STATE_DAILY;
bool newPacketReceived = true; // Flag to trigger a redraw

// -- Buffers to hold received data --
RTC_DATA_ATTR uint8_t values1d[NUM_READINGS];
RTC_DATA_ATTR uint8_t values1w[NUM_READINGS];
RTC_DATA_ATTR uint8_t values1m[NUM_READINGS];
RTC_DATA_ATTR uint8_t lastTankPercent = 255; // 255 = unknown

// -- LoRa & Radio state --
static RadioEvents_t RadioEvents;
bool lora_idle = true;

// -- Button debouncing --
unsigned long lastDebounceTime = 0;

// =================================================================
// ==                 FUNCTION PROTOTYPES                         ==
// =================================================================

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void handleButtonPress();
void updateDisplay();
void drawAxes();
void drawPlot(const uint8_t* values);
void drawTank(uint8_t percent);
void drawPlotTitle(const char* title);


// =================================================================
// ==                       SETUP & LOOP                          ==
// =================================================================

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize display
  display.setRotation(270);
  DRAW(display) {
    display.print("Waiting for data...");
  }

  // Initialize Radio
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODING_RATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
}

void loop() {
  // Always keep LoRa radio in RX mode or processing interrupts
  if (lora_idle) {
    lora_idle = false;
    Serial.println("Starting RX mode...");
    Radio.Rx(0); // 0 = continuous RX
  }
  Radio.IrqProcess();

  // Check for button press to change state
  handleButtonPress();

  // If new data arrived or the state changed, update the screen
  if (newPacketReceived) {
    updateDisplay();
    newPacketReceived = false;
  }
}

// =================================================================
// ==                   LORA EVENT HANDLERS                       ==
// =================================================================

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  lora_idle = true; // Go back to RX mode in the next loop
  
  // Basic packet validation
  if (size < 2 || size > BUFFER_SIZE) {
    Serial.println("Received invalid packet size.");
    return;
  }

  // Parse the packet identifier
  uint8_t packetId = payload[0];
  lastTankPercent = payload[1];
  const uint8_t* dataValues = payload + 2;
  uint16_t dataSize = size - 2;

  Serial.printf("Received packet ID 0x%X with RSSI %d\n", packetId, rssi);

  // Copy data to the correct buffer based on the identifier
  switch (packetId) {
    case ID_DAILY:
      memcpy(values1d, dataValues, dataSize);
      currentDisplayState = STATE_DAILY;
      break;
    case ID_WEEKLY:
      memcpy(values1w, dataValues, dataSize);
      currentDisplayState = STATE_WEEKLY;
      break;
    case ID_MONTHLY:
      memcpy(values1m, dataValues, dataSize);
      currentDisplayState = STATE_MONTHLY;
      break;
    default:
      Serial.println("Unknown packet ID.");
      return; // Do nothing if the packet is unknown
  }

  // Set a flag to tell the main loop to redraw the screen
  newPacketReceived = true; 
}

// =================================================================
// ==                     USER INTERFACE                          ==
// =================================================================

void handleButtonPress() {
  if (digitalRead(BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    
    // Cycle to the next state
    if (currentDisplayState == STATE_DAILY)      currentDisplayState = STATE_WEEKLY;
    else if (currentDisplayState == STATE_WEEKLY) currentDisplayState = STATE_MONTHLY;
    else                                          currentDisplayState = STATE_DAILY;

    newPacketReceived = true; // Trigger a redraw to show the new view
    lastDebounceTime = millis();
    Serial.printf("Button press -> New State: %d\n", currentDisplayState);
  }
}

void updateDisplay() {
  Serial.println("Updating display...");
  display.clearMemory(); // Clear the buffer before drawing
  drawAxes();
  drawTank(lastTankPercent);

  switch (currentDisplayState) {
    case STATE_DAILY:
      drawPlot(values1d);
      drawPlotTitle("Last Day");
      break;
    case STATE_WEEKLY:
      drawPlot(values1w);
      drawPlotTitle("Last Week");
      break;
    case STATE_MONTHLY:
      drawPlot(values1m);
      drawPlotTitle("Last Month");
      break;
  }
  
  display.update(); // Push the buffer to the physical screen
  Serial.println("Display update complete.");
}

// =================================================================
// ==                     DRAWING HELPERS                         ==
// =================================================================
// NOTE: "Magic numbers" for coordinates and sizes are kept here for now,
// but could be defined as constants at the top for even more clarity.

void drawPlotTitle(const char* title) {
    display.setCursor(70, 120);
    display.print(title);
}

void drawAxes() {
  display.drawRect(24, 14, NUM_READINGS, 101, BLACK);
  display.drawLine(19, 64, 24, 64, BLACK);
  display.setCursor(1, 60);
  display.print(" 50");
  display.drawLine(19, 64 - 50, 24, 64 - 50, BLACK);
  display.setCursor(1, 60 - 50);
  display.print("100");
  display.drawLine(19, 64 + 50, 24, 64 + 50, BLACK);
  display.setCursor(1, 60 + 50);
  display.print("  0");
  display.drawLine(21, 64 - 25, 24, 64 - 25, BLACK);
  display.setCursor(1, 60 - 25);
  display.print(" 75");
  display.drawLine(21, 64 + 25, 24, 64 + 25, BLACK);
  display.setCursor(1, 60 + 25);
  display.print(" 25"); 
}

void drawPlot(const uint8_t* values) {
  const uint8_t offset_x = 24;
  const uint8_t offset_y = 14;

  for (int i = 1; i < NUM_READINGS; i++) {
    if (values[i] > 100 || values[i-1] > 100)
      continue;
    
    // If the previous point was invalid, just draw a dot
    if (values[i - 1] == 0) {
      display.drawPixel(i + offset_x, 100 - values[i] + offset_y, BLACK);
    } else {
      display.drawLine(i - 1 + offset_x, 100 - values[i - 1] + offset_y,
                       i + offset_x, 100 - values[i] + offset_y, BLACK);
    }
  }
}

void drawTank(uint8_t percent) {
  #if PRINTTOSERIAL == 1
    Serial.printf("Last percentage: %i\n", percent);
  #endif
  String text;
  if (percent == 255) {
    text = "Fail";
  } else if (percent > 100) {
    text = "???";
  } else if (percent == 100) {
    text = "100%"; // Consistent width
  } else {
    text = String(percent) + " %";
  }
  
  display.drawRect(208, 60, 74, 50, BLACK);
  if (percent <= 100) {
    display.fillRect(208, 60 + 50 - (percent / 2), 74, (percent / 2), BLACK);
  }
  
  display.setTextSize(3);
  display.setCursor(214, 30);
  display.print(text);
  display.setTextSize(1);
}
