/*
This file is based on main/sd_card_example_main.c in the following directory:
https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card
The file 'sd_card_example_main.c' is modified to adapt to 
MH-ET Live ESP32 minikit (http://forum.mhetlive.com/topic/8/mh-et-live-minikit-for-esp32)
and WeMos Micro SD Card Shield	(https://wiki.wemos.cc/products:d1_mini_shields:micro_sd_card_shield).
*/

/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

#include "esp_timer.h"


// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:


// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#if CONFIG_SDSPI
// Pin mapping when using SPI mode.
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2

//#define PIN_NUM_MISO 2
#define PIN_NUM_MISO 16
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS	 13

#elif CONFIG_IDF_TARGET_ESP32C3
#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 9
#define PIN_NUM_CLK  8
#define PIN_NUM_CS	 19

#endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2


#if CONFIG_IDF_TARGET_ESP32S2 ||CONFIG_IDF_TARGET_ESP32C3
#define SPI_DMA_CHAN	host.slot
#else
#define SPI_DMA_CHAN	1
#endif

#endif //CONFIG_SDSPI

#define TIME_ARRAY_SIZE		 10000
#define WRITE_BUFFER_SIZE	(16*1024)
#define PRINT_DIFF			0

uint64_t time_array[TIME_ARRAY_SIZE];

char	write_buffer[WRITE_BUFFER_SIZE];

void sdBenchi(size_t writeSize) {
	// Check if destination file exists before writing
	struct stat st;
	if (stat("/sdcard/hello.txt", &st) == 0) {
		// Delete it if it exists
		unlink("/sdcard/hello.txt");
	}

	FILE* f = fopen("/sdcard/hello.txt", "w");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for writing");
		return;
	}

	// Get time in microseconds since boot.
	uint64_t start = esp_timer_get_time();
	for ( int counter = 0 ; counter < TIME_ARRAY_SIZE ; counter++) {
		fwrite(write_buffer, 1, writeSize, f);
		time_array[counter] = esp_timer_get_time();
	}
	fclose(f);
	ESP_LOGD(TAG, "File written");

	uint64_t sum = 0;
	uint64_t maximum = 0;
	uint64_t minimum = UINT64_MAX;
	for ( int i=0 ; i < TIME_ARRAY_SIZE; i++ ) {
		uint64_t end =	time_array[i];
		uint64_t diff = end - start;
#if PRINT_DIFF
		printf("%d: start=%llu, end=%llu, diff=%llu\n", i, start, end, diff);
#endif
		maximum = (diff > maximum) ? diff : maximum;
		minimum = (diff < minimum) ? diff : minimum;
		sum += diff;
		start = end;
	}
	//uint64_t average = sum / TIME_ARRAY_SIZE;
	printf("write buffer size = %d\n", writeSize);
	printf("maximum = %lld millsec.\n", maximum/1000);
	//printf("average = %lld millsec.\n", average/1000);
	printf("minimum = %lld millsec.\n", minimum/1000);
}

void app_main(void)
{
	esp_err_t ret;

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.format_if_mount_failed = true,
#else
		.format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};

#if CONFIG_SDMMC

	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.

	ESP_LOGI(TAG, "Using SDMMC peripheral");
	sdmmc_host_t host = SDMMC_HOST_DEFAULT();

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	// To use 1-line SD mode, change this to 1:
	slot_config.width = 4;

	// On chips where the GPIOs used for SD card can be configured, set them in
	// the slot_config structure:
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
	slot_config.clk = GPIO_NUM_14;
	slot_config.cmd = GPIO_NUM_15;
	slot_config.d0 = GPIO_NUM_2;
	slot_config.d1 = GPIO_NUM_4;
	slot_config.d2 = GPIO_NUM_12;
	slot_config.d3 = GPIO_NUM_13;
#endif

	// Enable internal pullups on enabled pins. The internal pullups
	// are insufficient however, please make sure 10k external pullups are
	// connected on the bus. This is for debug / example purpose only.
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
					 "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
					 "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return;
	}

#endif // SDMMC

#if CONFIG_SDSPI

	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.
	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return;
	}

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;

	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
					 "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
					 "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return;
	}
#endif // SDSPI


	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	// Use POSIX and C standard library functions to work with files.
	// First create a file.


	ESP_LOGI(TAG, "Opening file");
	FILE* f = fopen("/sdcard/hello.txt", "w");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for writing");
		return;
	}


	// initialize write buffer
	for ( int i = 0 ; i < WRITE_BUFFER_SIZE ; i++ ) {
		write_buffer[i] = ' ' + (i % 64);
	}

	
	uint64_t start = esp_timer_get_time();
	for ( int counter = 0 ; counter < TIME_ARRAY_SIZE ; counter++) {
		fwrite(write_buffer, 1, WRITE_BUFFER_SIZE, f);
		time_array[counter] = esp_timer_get_time();
	}
	fclose(f);
	ESP_LOGI(TAG, "File written");

	uint64_t sum = 0;
	uint64_t maximum = 0;
	uint64_t minimum = UINT64_MAX;
	for ( int i=0 ; i < TIME_ARRAY_SIZE; i++ ) {
		uint64_t end =	time_array[i];		  
		uint64_t diff = end - start;
#if PRINT_DIFF
		printf("%d: start=%llu, end=%llu, diff=%llu\n", i, start, end, diff);
#endif
		maximum = (diff > maximum) ? diff : maximum;
		minimum = (diff < minimum) ? diff : minimum;
		sum += diff;
		start = end;
	}
	uint64_t average = sum / TIME_ARRAY_SIZE;
	printf("write buffer size = %d\n", WRITE_BUFFER_SIZE);
	printf("sum=%llu microseconds, average=%llu microseconds\n", sum, average);
	printf("maximum=%llu microseconds, minimum=%llu microseconds\n", maximum, minimum);
	printf("highest write speed = %llu byte/s\n", ((uint64_t)WRITE_BUFFER_SIZE)*1000*1000/minimum);
	printf("average write speed = %llu byte/s\n", ((uint64_t)WRITE_BUFFER_SIZE)*1000*1000/average);
	printf("lowest write speed = %llu byte/s\n", ((uint64_t)WRITE_BUFFER_SIZE)*1000*1000/maximum);


	sdBenchi(1024*1);
	sdBenchi(1024*2);
	sdBenchi(1024*4);
	sdBenchi(1024*8);

#if 0
	// Check if destination file exists before renaming
	struct stat st;
	if (stat("/sdcard/foo.txt", &st) == 0) {
		// Delete it if it exists
		unlink("/sdcard/foo.txt");
	}

	// Rename original file
	ESP_LOGI(TAG, "Renaming file");
	if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
		ESP_LOGE(TAG, "Rename failed");
		return;
	}

	// Open renamed file for reading
	ESP_LOGI(TAG, "Reading file");
	f = fopen("/sdcard/foo.txt", "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for reading");
		return;
	}
	char line[64];
	fgets(line, sizeof(line), f);
	fclose(f);
	// strip newline
	char* pos = strchr(line, '\n');
	if (pos) {
		*pos = '\0';
	}
	ESP_LOGI(TAG, "Read from file: '%s'", line);
#endif

	// All done, unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdmmc_unmount();
	ESP_LOGI(TAG, "Card unmounted");
}
