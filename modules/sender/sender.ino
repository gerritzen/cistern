#include <SPI.h>
#include <LoRa.h> /* https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md */

int counter = 0;

void setup() {

  uint16_t startTime;
  Serial.begin( 9600 );
  startTime = millis();
  while (!Serial) { /* that alone would stall the system if no serial connection is present :( */
    if ((millis() - startTime) >= 2000) { /* if it takes more than 2seconds to find the Serial, we'll do without it. */
      break;
    }
  }

  Serial.println("LoRa Sender");

  LoRa.setPins(D6, D5, D4);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
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

  // send packet
  LoRa.beginPacket();
  LoRa.print("gnihi  ");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  delay(3000);
}
