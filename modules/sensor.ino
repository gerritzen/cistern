#include <SoftwareSerial.h> // EspSoftwareSerial

SoftwareSerial mySerial(D7, D6); // RX, TX

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  mySerial.write(0x01);
  delay(50);
  if (mySerial.available()) {
    uint16_t distance;
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
  }
  delay(1000);
}
