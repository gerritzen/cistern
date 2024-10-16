# README
Small Arduino programs for testing individual aspects of the project

# AJ-SR04M
Documentation is sparse. I did not manage to operate the sensor in the normal mode. By installing a 47kΩ resistor in R19, the sensor works in "mode 4: low power serial mode".
Sending `0x01` triggers a measurement which is returned as 4 bytes:

1. Start byte `0xFF`
2. Upper data byte
3. Lower data byte
4. Checksum

The value is in millimetres.
A note on the checksum: According to code I found online, it is supposed to be `(buf[2] + buf[3]) & 0xFF == buf[4]`, but I found that it is always off by 1. So, I use `(buf[2] + buf[3] - 1) & 0xFF == buf[4]`.
