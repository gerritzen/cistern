// Just send NUM_READINGS with the i-th reading set to i+10 for debugging.

#include <SPI.h>
#include <LoRa.h> /* https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md */
#include <SoftwareSerial.h> // EspSoftwareSerial
#include "../definitions.h"

SoftwareSerial mySerial(D3, D7); // RX, TX

int counter = 0;
RTC_DATA_ATTR uint8_t ringbuffer[NUM_READINGS];
RTC_DATA_ATTR int8_t nextIndex = 0;
RTC_DATA_ATTR bool firstBoot = true;

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
      ringbuffer[i] = 0xFF;
    }
    firstBoot = false;
  }
}


void loop() {
  //  Serial.print("Sending packet: ");
  //  Serial.print(counter);
  //  Serial.print(": ");
  //  Serial.println(readDistance());
  for (int i = 0; i < NUM_READINGS; i++)
    ringbuffer[i] = 10 + i;
  // send packet
  Serial.println(NUM_READINGS);
  dumpBufferToSerial();
  LoRa.beginPacket();
  //  LoRa.print(counter);
  //  LoRa.print(": distance ");
  //  Serial.println(distance);
  //  LoRa.write("0x38");
  dumpBufferToLora();
  //  LoRa.print(readDistance());
  LoRa.endPacket();
  //  counter++;
  //  sleep(2);
delay(10000);
}

void sleepSeconds(int s) {
  //  Serial.printf("Sleeping for %d s.\n", s);
  LoRa.sleep();
  esp_sleep_enable_timer_wakeup(s * 1e6);
  esp_deep_sleep_start();
}

void push_back(uint8_t d) {
  ringbuffer[nextIndex] = d;
  nextIndex = (nextIndex + 1) % NUM_READINGS;
}

void dumpBufferToSerial() {
  Serial.println("Buffer dump:");
  for (int i = nextIndex; i != (nextIndex - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    Serial.print(ringbuffer[i], DEC);
    Serial.print(", ");
  }
  Serial.println(ringbuffer[(nextIndex - 1 + NUM_READINGS) % NUM_READINGS]);
}

void dumpBufferToLora() {
  // needs   LoRa.beginPacket(); and endPacket()!
  for (int i = nextIndex; i != (nextIndex - 1 + NUM_READINGS) % NUM_READINGS; (i = (i + 1) % NUM_READINGS)) {
    LoRa.write(ringbuffer[i]);
  }
  LoRa.write(ringbuffer[(nextIndex - 1 + NUM_READINGS) % NUM_READINGS]);
}
