/*
 * Board: ESP32 Arduino -> XIAO_ESP32C6
 */

#include <SPI.h>
#include <LoRa.h> /* https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md */
#include <SoftwareSerial.h> // EspSoftwareSerial
#include "../definitions.h"
#define DEEPSLEEP 0
#define PRINTTOSERIAL 1

SoftwareSerial mySerial(D3, D7); // RX, TX

int counter = 0;
RTC_DATA_ATTR uint8_t ringbuffer1d[NUM_READINGS];
RTC_DATA_ATTR uint8_t ringbuffer1w[NUM_READINGS];
RTC_DATA_ATTR uint8_t ringbuffer1m[NUM_READINGS];
RTC_DATA_ATTR uint16_t nextIndex1d = 0;
RTC_DATA_ATTR uint16_t nextIndex1w = 0;
RTC_DATA_ATTR uint16_t nextIndex1m = 0;
RTC_DATA_ATTR bool firstBoot = true;
RTC_DATA_ATTR uint32_t wakeupCounter = 0;

void setup() {

  uint16_t startTime;
  Serial.begin(115200);
  mySerial.begin(9600);
  startTime = millis();

  Serial.println("LoRa Sender");

  LoRa.setPins(D6, D5, D4);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    ESP.restart();
  }
  LoRa.setTxPower(2); /* TX power in dB, 2 to 20, defaults to 17 */
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR); /* max spread >> max range */
  /* Supported values are between 6 and 12. If a spreading factor of 6 is set, implicit header mode must be used to transmit and receive packets, defaults to 7 */

  /* Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. */
  /* defaults to 125E3 */
  /* choosing the lower to increase range */
  // LoRa.setSignalBandwidth(7.8E3);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING_RATE); /* 4/8 Supported values are between 5 and 8, default is 5 */
  LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH); /* Supported values are between 6 and 65535, default is 8 */

  if (firstBoot) {
    // Initialize array to unreasonable values
    for (int i = 0; i < NUM_READINGS; i++) {
      ringbuffer1d[i] = 0xFF;
      ringbuffer1w[i] = 0xFF;
      ringbuffer1m[i] = 0xFF;
    }
    firstBoot = false;
  }
}


void loop() {
  uint16_t distance = readDistance();

  if (wakeupCounter % 3 == 0) {
    push_back(to8b(distance), ringbuffer1d, &nextIndex1d);
    push_back(to8b(distance)+1, ringbuffer1d, &nextIndex1d);
    push_back(to8b(distance), ringbuffer1d, &nextIndex1d);
    push_back(to8b(distance)-1, ringbuffer1d, &nextIndex1d);
  }
  
  if (wakeupCounter % 20 == 0) {
    push_back(to8b(distance), ringbuffer1w, &nextIndex1w);
    push_back(to8b(distance), ringbuffer1w, &nextIndex1w);
    push_back(to8b(distance), ringbuffer1w, &nextIndex1w);
    push_back(to8b(distance), ringbuffer1w, &nextIndex1w);
  }
  
  if (wakeupCounter % 80 == 0) {
    push_back(to8b(distance), ringbuffer1m, &nextIndex1m);
    push_back(to8b(distance)+2, ringbuffer1m, &nextIndex1m);
    push_back(to8b(distance), ringbuffer1m, &nextIndex1m);
    push_back(to8b(distance)-2, ringbuffer1m, &nextIndex1m);
  }

  if (wakeupCounter % 3 == 0) {
    LoRa.beginPacket();
    if (wakeupCounter % 9 == 0) {
      LoRa.write(ID_DAILY);
      LoRa.write(to8b(distance));
      dumpBufferToLora(ringbuffer1d, &nextIndex1d);
      #if PRINTTOSERIAL == 1
      Serial.println("Day:");
      dumpBufferToSerial(ringbuffer1d, &nextIndex1d);
      #endif
    } else if (wakeupCounter % 9 == 3) {
      LoRa.write(ID_WEEKLY);
      LoRa.write(to8b(distance));
      dumpBufferToLora(ringbuffer1w, &nextIndex1w);
      #if PRINTTOSERIAL == 1
      Serial.println("Week:");
      dumpBufferToSerial(ringbuffer1w, &nextIndex1w);
      #endif
    } else if (wakeupCounter % 9 == 6) {
      LoRa.write(ID_MONTHLY);
      LoRa.write(to8b(distance));
      dumpBufferToLora(ringbuffer1m, &nextIndex1m);
      #if PRINTTOSERIAL == 1
      Serial.println("Month:");
      dumpBufferToSerial(ringbuffer1m, &nextIndex1m);
      #endif

    }
    LoRa.endPacket();
  }
    //  counter++;
    //  sleep(2);
  #if PRINTTOSERIAL == 1
  Serial.print("Wakeup Counter: ");
  Serial.println(wakeupCounter);
  #endif
  wakeupCounter++;
  sleepSeconds(3); // 180
}

uint16_t readDistance() {
  uint16_t distance = -1;
  mySerial.write(0x01);
  delay(50);
  if (mySerial.available()) {
    byte buf[4];

    mySerial.readBytes(buf, 4);
    if (buf[0] == 255) {
      distance = (buf[1] << 8) + buf[2];
      if (((buf[1] + buf[2] - 1) & 0xFF) != buf[3]) {
        //        Serial.println("Invalid result");
        return 6016;
      }
      else if (distance == 6016) {
#if PRINTTOSERIAL == 1
                Serial.println("Distance [mm]: NaN");
#endif
      } else {
#if PRINTTOSERIAL == 1
                Serial.print("Distance [mm]: ");
                Serial.println(distance);
#endif
      }
    }
  } else {
    //    Serial.println("No mySerial");
  }
  return (distance);
}

void sleepSeconds(int s) {
  //  Serial.printf("Sleeping for %d s.\n", s);
#if DEEPSLEEP == 1
  LoRa.sleep();
  esp_sleep_enable_timer_wakeup(s * 1e6);
  esp_deep_sleep_start();
#else
  sleep(s);
#endif
}

uint8_t to8b(uint16_t d) {
  // Smallest reading is like 198 or so
  if (d < 200)
    return 100;
  // Errors are shown as this number
  if (d == 6016)
    return 0xFF;
  if (d > DEPTH_CISTERN)
    return 0;
  return (DEPTH_CISTERN - d) / (DEPTH_CISTERN / 100);
}

void push_back(uint8_t d, uint8_t* buf, uint16_t* index) {
  buf[*index] = d;
  *index = (*index + 1) % NUM_READINGS;
}

void dumpBufferToSerial(uint8_t* buf, uint16_t* index) {
  Serial.println("Buffer dump:");
  for (int i = *index; i != (*index - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    Serial.print(buf[i], DEC);
    Serial.print(", ");
  }
  Serial.println(buf[(*index - 1 + NUM_READINGS) % NUM_READINGS]);
}

void dumpBufferToLora(uint8_t* buf, uint16_t* index) {
  // needs   LoRa.beginPacket(); and endPacket()!
  for (int i = *index; i != (*index - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    LoRa.write(buf[i]);
  }
  LoRa.write(buf[(*index - 1 + NUM_READINGS) % NUM_READINGS]);
}
