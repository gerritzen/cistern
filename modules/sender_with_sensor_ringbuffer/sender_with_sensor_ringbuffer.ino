/*
 * Board: ESP32 Arduino -> XIAO_ESP32C6
 * This code reads a sensor, logs the data into daily, weekly, and monthly
 * ring buffers, and transmits the data via LoRa on a staggered schedule.
 * It is designed to use deep sleep for power efficiency.
 */

#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h> // EspSoftwareSerial
#include <string.h>         // For memset
#include "../definitions.h"

// =================================================================
// ==                      CONFIGURATION                          ==
// =================================================================

// -- Timing Configuration --
// The "base tick" of the system, in seconds. The device wakes up, runs the loop, then sleeps for this duration.
const int DEEP_SLEEP_SECONDS = 180; // 3 minutes

// Intervals are defined in "ticks". For example, if DEEP_SLEEP_SECONDS is 180 (3 min):
// DAILY_LOG_INTERVAL_TICKS = 3 means logging happens every 3 * 3 = 9 minutes.
const int DAILY_LOG_INTERVAL_TICKS   = 3;  // Log every ~9 minutes
const int WEEKLY_LOG_INTERVAL_TICKS  = 20; // Log every 1 hour
const int MONTHLY_LOG_INTERVAL_TICKS = 80; // Log every 4 hours

// -- Pin Definitions --
const int LORA_SS_PIN   = D6;
const int LORA_RST_PIN  = D5;
const int LORA_DIO0_PIN = D4;
const int SERIAL_RX_PIN = D3;
const int SERIAL_TX_PIN = D7;

// =================================================================
// ==                      GLOBAL VARIABLES                       ==
// =================================================================

// -- Software Serial for Sensor --
SoftwareSerial mySerial(SERIAL_RX_PIN, SERIAL_TX_PIN);

// -- RTC (Deep Sleep) Variables --
RTC_DATA_ATTR uint8_t ringbuffer1d[NUM_READINGS];
RTC_DATA_ATTR uint8_t ringbuffer1w[NUM_READINGS];
RTC_DATA_ATTR uint8_t ringbuffer1m[NUM_READINGS];
RTC_DATA_ATTR uint16_t nextIndex1d = 0;
RTC_DATA_ATTR uint16_t nextIndex1w = 0;
RTC_DATA_ATTR uint16_t nextIndex1m = 0;
RTC_DATA_ATTR bool firstBoot = true;
RTC_DATA_ATTR uint32_t wakeupCounter = 0;

// =================================================================
// ==                   FUNCTION PROTOTYPES                       ==
// =================================================================
// (Forward declaration of functions for clarity)
uint16_t readDistance();
uint8_t to8b(uint16_t d);
void sleepSeconds(int s);
void push_back(uint8_t d, uint8_t* buf, uint16_t* index);
void dumpBufferToSerial(uint8_t* buf, uint16_t* index);
void dumpBufferToLora(uint8_t* buf, uint16_t* index);

// =================================================================
// ==                       SETUP & LOOP                          ==
// =================================================================

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  Serial.println("\n--- LoRa Sender Booting Up ---");

  // Initialize LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("FATAL: Starting LoRa failed!");
    delay(1000);
    ESP.restart();
  }
  LoRa.setTxPower(2);
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);

  // One-time initialization of buffers on first ever boot
  if (firstBoot) {
    Serial.println("First boot: Initializing ring buffers.");
    memset(ringbuffer1d, 0xFF, NUM_READINGS);
    memset(ringbuffer1w, 0xFF, NUM_READINGS);
    memset(ringbuffer1m, 0xFF, NUM_READINGS);
    firstBoot = false;
  }
}

void loop() {
  uint16_t distance = readDistance();
  uint8_t currentLevel = to8b(distance);

  if (wakeupCounter % DAILY_LOG_INTERVAL_TICKS == 0) {
    push_back(currentLevel, ringbuffer1d, &nextIndex1d);
  }
  if (wakeupCounter % WEEKLY_LOG_INTERVAL_TICKS == 0) {
    push_back(currentLevel, ringbuffer1w, &nextIndex1w);
  }
  if (wakeupCounter % MONTHLY_LOG_INTERVAL_TICKS == 0) {
    push_back(currentLevel, ringbuffer1m, &nextIndex1m);
  }

  uint8_t packetId = 0;
  uint8_t* bufferToDump = nullptr;
  uint16_t* indexToDump = nullptr;

  int cycleStep = wakeupCounter % 3;

  if (cycleStep == 0) {
    // On the 1st tick of the cycle, send Daily
    packetId = ID_DAILY;
    bufferToDump = ringbuffer1d;
    indexToDump = &nextIndex1d;
  } else if (cycleStep == 1) {
    // On the 2nd tick, send Weekly
    packetId = ID_WEEKLY;
    bufferToDump = ringbuffer1w;
    indexToDump = &nextIndex1w;
  } else { // cycleStep == 2
    // On the 3rd tick, send Monthly
    packetId = ID_MONTHLY;
    bufferToDump = ringbuffer1m;
    indexToDump = &nextIndex1m;
  }
  
  
  // Build and send the selected packet
  #if PRINTTOSERIAL == 1
    Serial.printf("Transmitting packet type 0x%X...\n", packetId);
  #endif
  LoRa.beginPacket();
  LoRa.write(packetId);
  LoRa.write(currentLevel); // Send the latest reading
  dumpBufferToLora(bufferToDump, indexToDump);
  LoRa.endPacket();

  // --- 3. Prepare for Sleep ---
  wakeupCounter++;
  #if PRINTTOSERIAL == 1
    Serial.printf("Wakeup Counter: %u. Sleeping for %d seconds...\n", wakeupCounter, DEEP_SLEEP_SECONDS);
    Serial.println("---------------------------------");
  #endif
  sleepSeconds(DEEP_SLEEP_SECONDS);
}

// =================================================================
// ==                    HELPER FUNCTIONS                         ==
// =================================================================

uint16_t readDistance() {
  uint16_t distance = 6016;
  mySerial.write(0x01);
  delay(50);
  if (mySerial.available()) {
    byte buf[4];
    mySerial.readBytes(buf, 4);
    if (buf[0] == 255) {
      distance = (buf[1] << 8) + buf[2];
      if (((buf[1] + buf[2] - 1) & 0xFF) != buf[3]) {
        return 6016; // Checksum error
      }
    }
  }
  #if PRINTTOSERIAL == 1
    Serial.printf("Distance reading: %d mm\n", distance);
  #endif
  return distance;
}

uint8_t to8b(uint16_t d) {
  if (d < 200) return 100;
  if (d == 6016) return 0xFF; // Error value
  if (d > DEPTH_CISTERN) return 0;
  return (uint8_t)((DEPTH_CISTERN - d) / (DEPTH_CISTERN / 100.0));
}

void sleepSeconds(int s) {
#if DEEPSLEEP == 1
  LoRa.sleep();
  esp_sleep_enable_timer_wakeup(s * 1000000ULL);
  esp_deep_sleep_start();
#else
  delay(s * 1000);
#endif
}

void push_back(uint8_t d, uint8_t* buf, uint16_t* index) {
  buf[*index] = d;
  *index = (*index + 1) % NUM_READINGS;
}

void dumpBufferToLora(uint8_t* buf, uint16_t* index) {
  for (int i = *index; i != (*index - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    LoRa.write(buf[i]);
  }
  LoRa.write(buf[(*index - 1 + NUM_READINGS) % NUM_READINGS]);
}

void dumpBufferToSerial(uint8_t* buf, uint16_t* index) {
  Serial.print("Buffer Dump: ");
  for (int i = *index; i != (*index - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    Serial.printf("%d,", buf[i]);
  }
  Serial.printf("%d\n", buf[(*index - 1 + NUM_READINGS) % NUM_READINGS]);
}
