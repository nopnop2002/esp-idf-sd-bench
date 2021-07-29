# esp-idf-sd-bench
sd card benchmark for esp-idf.   
I wanted to know the writing performance of the SD card.   
I modified [this](https://github.com/kunsen-an/espidf_sd_card_write_test).   

# Installation for ESP32
```
git clone https://github.com/nopnop2002/esp-idf-sd-bench
cd esp-idf-sd-bench/
idf.py set-target esp32
idf.py menuconfig
idf.py flash
```


# Configuration

![config-main](https://user-images.githubusercontent.com/6020549/127577532-f6522d74-224c-4b29-85ab-b7fd1ad58e7a.jpg)

For SDSPI   
![config-spi](https://user-images.githubusercontent.com/6020549/127577535-99519b3d-d662-4109-abe1-6b716265ec20.jpg)

For SDMMC   
![config-sdmmc](https://user-images.githubusercontent.com/6020549/127577538-60e06e7a-5909-490c-854d-7fe0b8d1482e.jpg)


# Wireing for SDSPI
```
ESP32 pin     | SD card pin | SPI pin | Notes
--------------|-------------|---------|------------
GPIO16        | D0          | MISO    |
GPIO13 (MTCK) | D3          | CS      | 
GPIO14 (MTMS) | CLK         | SCK     | 
GPIO15 (MTDO) | CMD         | MOSI    | 10k pullup 
```

# Wireing for SDMMC
```
ESP32 pin     | SD card pin | Notes
--------------|-------------|------------
GPIO14 (MTMS) | CLK         | 10k pullup in SD mode
GPIO15 (MTDO) | CMD         | 10k pullup in SD mode
GPIO2         | D0          | 10k pullup in SD mode, pull low to go into download mode (see Note about GPIO2 below!)
GPIO4         | D1          | not used in 1-line SD mode; 10k pullup in 4-line SD mode
GPIO12 (MTDI) | D2          | not used in 1-line SD mode; 10k pullup in 4-line SD mode (see Note about GPIO12 below!)
GPIO13 (MTCK) | D3          | not used in 1-line SD mode, but card's D3 pin must have a 10k pullup
```

# Benchmark

|SoC|Freq(Mhz)|Interface|Highest|Lowest|
|:-:|:-:|:-:|:-:|:-:|
|ESP32|160|SDSPI|1282.6 Kbyte/s|55.8 Kbyte/s|
|ESP32|240|SDSPI|1462.0 Kbyte/s|256.9 Kbyte/s|
|ESP32|160|SDMMC|5136.0 Kbyte/s|121.4 Kbyte/s|
|ESP32|240|SDMMC|5310.8 Kbyte/s|115.7 Kbyte/s|
