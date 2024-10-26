#include <SPI.h>
#include <LoRa.h> /* https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md */
#include <SoftwareSerial.h> // EspSoftwareSerial

SoftwareSerial mySerial(D3, D7); // RX, TX

int counter = 0;

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
  LoRa.setTxPower(20); /* TX power in dB, 2 to 20, defaults to 17 */
  LoRa.setSpreadingFactor(7); /* max spread >> max range */
  /* Supported values are between 6 and 12. If a spreading factor of 6 is set, implicit header mode must be used to transmit and receive packets, defaults to 7 */

  /* Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. */
  /* defaults to 125E3 */
  /* choosing the lower to increase range */
  // LoRa.setSignalBandwidth(7.8E3);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(6); /* 4/8 Supported values are between 5 and 8, default is 5 */
  LoRa.setPreambleLength(8); /* Supported values are between 6 and 65535, default is 8 */
}


void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  Serial.println(readDistance());
  // send packet
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.print(": distance ");
  LoRa.print(readDistance());
  LoRa.endPacket();
  counter++;
  sleepSeconds(10);
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
        Serial.println("Invalid result");
      }
      else if (distance == 6016) {
        Serial.println("Distance [mm]: NaN");
      } else {
        Serial.print("Distance [mm]: ");
        Serial.println(distance);
      }
    }
  } else {
    Serial.println("No mySerial");
  }
  return (distance);
}

void sleepSeconds(int s) {
  Serial.printf("Sleeping for %d s.\n", s);
  LoRa.sleep();
  esp_sleep_enable_timer_wakeup(s * 1e6);
  esp_deep_sleep_start();
  //delay(10000);
  ESP.restart();
}
