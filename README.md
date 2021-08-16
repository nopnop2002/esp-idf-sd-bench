# esp-idf-sd-bench
sd card benchmark for esp-idf.   
I wanted to know the writing performance of the SD card.   
I modified [this](https://github.com/kunsen-an/espidf_sd_card_write_test).   

I used the ESP32-CAM development board.   
ESP32-CAM development board comes with an SDMMC card reader.   

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

__Note__   
Using an SDMMC card reader on a board other than the ESP32-CAM is quite difficult.   
It uses GPIO2 and GPIO12 to connect to the SDMMC card reader, both of which need to be pulled up.   
If GPIO2 is pulled up, some boards will be in UART Download mode.   
If the board has GPIO0, pulling up GPIO0 will force it into Flash boot mode.   
GPIO12 is used as a bootstrap pin to select the output voltage of the internal regulator that powers the flash chip (VDD_SDIO).   
On boards that use an internal regulator and a 3.3V flash chip, GPIO12 must be low on reset.   
SDMMC card readers cannot be used with such boards.   


On a board that uses an internal regulator and a 3.3V flash chip, this is displayed and it cannot be started.
```
rst:0x10 (RTCWDT_RTC_RESET),boot:0x3f (SPI_FAST_FLASH_BOOT)
flash read err, 1000
ets_main.c 371
ets Jun  8 2016 00:22:57
```

# Benchmark
The ESP32-CAM is equipped with an SDMMC card reader.   
I attached an external SPI card reader to the ESP32-CAM, used the same micro SD card, and the same development board, and measured under the same conditions.   

![ESP32-CAM-1](https://user-images.githubusercontent.com/6020549/127579321-129422ee-8210-46a8-831a-71a2b28de89a.JPG)

|SoC|Freq(Mhz)|Interface|Highest(Kbyte/s)|Lowest(Kbyte/s)|
|:-:|:-:|:-:|:-:|:-:|
|ESP32|160|SDSPI|1282.6|55.8|
|ESP32|240|SDSPI|1462.0|256.9|
|ESP32|160|SDMMC|5136.0|121.4|
|ESP32|240|SDMMC|5310.8|115.7|
