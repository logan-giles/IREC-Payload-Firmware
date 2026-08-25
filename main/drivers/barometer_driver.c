/*
 * barometer_driver.c
 *
 *  Created on: May 18, 2026
 *      Author: sir_l
 */


#include <bmp280.h>
#include "barometer_driver.h"
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "i2cdev.h"
#include "payload_data.h"

#define BARO_I2C_PORT I2C_NUM_0
#define BARO_SDA_PIN GPIO_NUM_20
#define BARO_SCL_PIN GPIO_NUM_21

static const char *TAG = "barometer";

static bmp280_t device; //Create baro "object"
static bmp280_params_t params; //Create parameter "object"
static bool initialized = false; //shows if BMP is ready to use

static float pressure_to_altitude_m(float pressure_pa){
	const float sea_level_pressure_pa = 101325.0f;
	return 44330.0f * (1.0f - powf(pressure_pa / sea_level_pressure_pa, 0.1903f));
}

bool barometer_init(void){
	esp_err_t err; //ESP-IDF's error/status type
	
	err = i2cdev_init(); //Initializes helper system that esp-idf-lib uses for I2C devices
	if(err != ESP_OK) {
		ESP_LOGE(TAG, "i2cdev_init failed: %s", esp_err_to_name(err)); //Prints an error message
		return false;
	}
	
	memset(&device, 0, sizeof(device)); //Initializes entire device variable to zero
	
	bmp280_init_default_params(&params);
	
	//Gives system bmp280 address, I2C and SDA/SCL port locations
	err = bmp280_init_desc(&device, BMP280_I2C_ADDRESS_1, BARO_I2C_PORT, BARO_SDA_PIN, BARO_SCL_PIN);
	
	if(err != ESP_OK){
		ESP_LOGE(TAG, "bmp280_init_desc failed %s", esp_err_to_name(err));
	}
	
	err = bmp280_init(&device, &params); //Configures and initializes the actual sensor
	if(err != ESP_OK){
		ESP_LOGE(TAG, "bmp280_init failed: %s", esp_err_to_name(err)); 
		return false;
	}
	
	initialized = true;
	ESP_LOGI(TAG, "BMP280 Initialized"); //Prints an informational message
	return true;
}

bool barometer_read(barometer_data_t *out){
	if(out == NULL || !initialized){
		return false;
	}
	
	//Variables to save read values in, then transfer these to our struct
	float temperature_c = 0.0f;
	float pressure_pa = 0.0f;
	
	esp_err_t err;
	
	err = bmp280_read_float(&device, &temperature_c, &pressure_pa, NULL);
	
	if(err != ESP_OK){
		ESP_LOGE(TAG, "bmp280_read_float failed %s", esp_err_to_name(err));
		out->valid = false;
		return false;
	}
	
	out->timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
	out->temperature_c = temperature_c;
	out->pressure_pa = pressure_pa;
	out->altitude_m = pressure_to_altitude_m(pressure_pa);
	out->valid = true;
	
	return true;
}



