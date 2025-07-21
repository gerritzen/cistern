# Readme
I picked up this project after 9 months of inactivity. Since things were in a working state, I wanted to create a backup of whatever firmware I had. For this, check the size of the flash:

```
esptool.py flash_id
```

In my case, it was 4MB, so I dumped it like

```
esptool.py --baud 115200 --port /dev/ttyACM0 read_flash 0x0 0x400000 fw-backup-sender-4M.bin
```

To restore, use the following:

```
esptool.py --baud 115200 --port /dev/ttyACM0 write_flash 0x0 fw-backup-sender-4M.bin
```
