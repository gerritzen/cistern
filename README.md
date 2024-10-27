# cistern
Monitoring of the water level in a remote cistern.

There are two sites, remote and local. On the remote site, I want to read
the water level of a cistern with an ultrasnonic distance sensor.
The value is sent to a local site (where people are) and can be read on a display.
Since the receiver is not always online, and the trend of the past 7 days should be
displayed, the past 168 hourly readings are stored on the cistern site and transmitted in a single package.

# Components
Cistern site:
- Xiao ESP32C6
- AJ-SR04M waterproof distance sensor
- LoRa module
- battery
- solar?

Readout:
- [Heltec Vision Master E290](https://docs.heltec.org/en/node/esp32/ht_vme290/index.html)

Backup:
- Xiao ESP32C6
- LoRa module


# Data format considerations
The data are displayed on a 296 x 128 pixel display.
So, even though the distance sensor has a resolution of ~13b, we can safely downsample
the readings to 8b before transmission.
That allows to send 168 readings in a single <255b LoRa packet without any bit packing shenanigans.


# Things to get to work
- [x] Waterproof sensor
- [ ] LoRa
- [ ] Battery charging and operation

