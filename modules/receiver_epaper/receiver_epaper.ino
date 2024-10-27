#include <heltec-eink-modules.h>
EInkDisplay_VisionMasterE290 display;


#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "../definitions.h"

#define RF_FREQUENCY                                433000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

// Shadows the one in ../definitions.h
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
// #define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
//#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 255 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
uint8_t values[NUM_READINGS];

static RadioEvents_t RadioEvents;

int16_t txNumber;

int16_t rssi, rxSize;

bool lora_idle = true;

void setup() {
  display.setRotation(270);                // Rotate 90deg clocklwise - more room
  DRAW (display) {
    display.print("LoRa Receiver!");
  }
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  txNumber = 0;
  rssi = 0;

  RadioEvents.RxDone = OnRxDone;
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                     LORA_CODING_RATE, 0, LORA_PREAMBLE_LENGTH,
                     LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                     0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}



void loop()
{
  if (lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  rssi = rssi;
  rxSize = size;
  memcpy(rxpacket, payload, size );
  rxpacket[size] = '\0';
  memcpy(values, payload, size);
  Radio.Sleep( );
  Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n", rxpacket, rssi, rxSize);
  for (int i = 0; i < size; i++) {
    Serial.print(rxpacket[i], DEC);
    Serial.print(" ");
  }
  //  DRAW (display) {
  //    display.print(rxpacket);
  //  }
  display.clearMemory();
  drawAxes();
  drawPlot();
  drawTank(values[NUM_READINGS - 1]);
  display.update();

  lora_idle = true;
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

void drawPlot() {
  for (int i = 1; i < NUM_READINGS; i++) {
    unsigned char offset_x = 24;
    unsigned char offset_y = 14;
    if (values[i] > 100 || values[i] == 0)
      continue;
    if (values[i - 1] > 100 || values[i - 1] == 0)
      display.drawPixel(i + offset_x, 100 - values[i] + offset_y, BLACK);
    else
      display.drawLine(i - 1 + offset_x, 100 - values[i - 1] + offset_y,
                       i + offset_x, 100 - values[i] + offset_y, BLACK);
  }
}

void drawTank(unsigned char percent) {
  String text = String(percent) + " %";
  if (percent > 100)
    text = "???";
  if (percent == 100)
    text = "100%"; // no space for consistent width
  display.drawRect(208, 60, 74, 50, BLACK);
  if (percent < 100)
    display.fillRect(208, 60 + 50 - percent / 2, 74, percent / 2, BLACK);
  display.setTextSize(3);
  display.setCursor(214, 30);
  display.print(text);
  display.setTextSize(1);

}
