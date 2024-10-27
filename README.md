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
So, even though the distance sensor has a resolution of ~10b (?), we can safely downsample
the readings to 8b before transmission.
That allows to send 168 readings in a single <255b LoRa packet without any bit packing shenanigans.

# How many times to measure the distance?
I noticed a huge variance in the sensor readings. If the variance was due to random, independent noise, measuring $n$ times and averaging the result would reduce the  Here's the variance by a factor $n$.
I find the standard deviation to be a more useful quantity, as it has the same dimension as the value itself.
We expect the standard deviation to go down with $\sqrt{n}$.
I made a small test stand made of a cardboard box to test the effect of averaging our measurements.

| n  | std. dev [mm] |
| -- | ------------- |
| 1  | 2.13          |
| 2  | 1.26          |
| 3  | 1.09          |
| 4  | 0.97          |
| 10 | 0.74          |

It is not an exact $1/\sqrt{n}$ behaviour, but that's ok.

Even at $n=1$, the values are typically contained within 4mm. Considering that it is a tradeoff between power draw and precision, I decided to only take single measurements for now.

# Things to get to work
- [x] Waterproof sensor
- [x] LoRa
- [x] Ring buffer
- [x] Displaying the values
- [ ] Battery charging and operation
- [ ] Battery percentage monitoring (requires soldering)

