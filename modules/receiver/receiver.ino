#include <SPI.h>
#include <LoRa.h> /* https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md */

String inString = ""; //hold incoming characters
String myMessage = ""; // hold complete message

int led = BUILTIN_LED;

int noSerial = 0;

void setup() {
  uint16_t startTime;
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  Serial.begin( 115200 );
  startTime = millis();

  if (!noSerial) {
    Serial.println("LoRa Receiver");
  }

  LoRa.setPins(D6, D5, D4);

  if (!LoRa.begin(433E6)) {
    if (!noSerial) {
      Serial.println("Starting LoRa failed!");
    }
    while (1);
  }
  LoRa.setGain(6); /* max RX gain */
  LoRa.setSpreadingFactor(12); /* max spread >> max range */
  /* Supported values are between 6 and 12. If a spreading factor of 6 is set, implicit header mode must be used to transmit and receive packets, defaults to 7 */

  /* Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. */
  /* defaults to 125E3 */
  /* choosing the lower to increase range */
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(6); /* 4/8 Supported values are between 5 and 8, default is 5 */
  LoRa.setPreambleLength(8); /* Supported values are between 6 and 65535, default is 8 */

  digitalWrite(led, LOW);
}

void loop() {
  String message = "";
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (!noSerial) {
      Serial.print("Received [" + String(packetSize) + "] : '");
    }

    // read packet
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    if (!noSerial) {
      Serial.print(message);
    }
    if (!noSerial) {
      Serial.print("' RSSI:" + String(LoRa.packetRssi()));
    }
    if (!noSerial) {
      Serial.println(" Snr:" + String(LoRa.packetSnr()));
    }

    digitalWrite(led, HIGH);
    delay(1500);
    digitalWrite(led, LOW);
  } else {
    Serial.println("Keine Post:(");
    delay(1000);
  }
}
