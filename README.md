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
- 4-line mode   
![config-sdmmc-4](https://user-images.githubusercontent.com/6020549/129650135-ba0efa8f-c094-4977-9a00-a1cf6184ed0a.jpg)
- 1-line mode   
![config-sdmmc-1](https://user-images.githubusercontent.com/6020549/129650160-0848fd68-356e-4308-bf4e-3fbc15703773.jpg)

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

### Note about GPIO2 (ESP32 only)
GPIO2 pin is used as a bootstrapping pin, and should be low to enter UART download mode.   
One way to do this is to turn off the SDMMC card reader when in UART download mode.   
Another way to do this is to connect GPIO0 and GPIO2 using a jumper, and then the auto-reset circuit on most development boards will pull GPIO2 low along with GPIO0, when entering download mode.

### Note about GPIO12 (ESP32 only)
Using the 4-line SD mode on the ESP32 is a bit tricky.   
GPIO12 is used as a bootstrap pin to select the output voltage of the internal regulator(VDD_SDIO).   
If GPIO12 is pulled up at reset, 1.8V will be output to VDD_SDIO.   
If GPIO12 is pulled down at reset, 3.3V will be output to VDD_SDIO.   
On boards that use an internal regulator(VDD_SDIO) and a 3.3V flash chip, GPIO12 must be low on reset.   
4-line SD mode cannot be used with such boards.   
In 1-line SD mode, GPIO12 is not used, so you can use the SDMMC card reader without any problems.

# Benchmark   
ESP32-CAM is equipped with an SDMMC card reader.   
With the ESP32-CAM, you can use 4-line SD mode without any problems.   
I attached an external SPI card reader to the ESP32-CAM, used the same micro SD card, and the same development board, and measured under the same conditions.   

![ESP32-CAM-1](https://user-images.githubusercontent.com/6020549/127579321-129422ee-8210-46a8-831a-71a2b28de89a.JPG)

|SoC|Freq(Mhz)|Interface|Highest(Kbyte/s)|Lowest(Kbyte/s)|
|:-:|:-:|:-:|:-:|:-:|
|ESP32|160|SDSPI|1282.6|55.8|
|ESP32|240|SDSPI|1462.0|256.9|
|ESP32|160|SDMMC(1-line)|2055.2|118.1|
|ESP32|240|SDMMC(1-line)|2076.3|365.3|
|ESP32|160|SDMMC(4-line)|5136.0|121.4|
|ESP32|240|SDMMC(4-line)|5310.8|115.7|
