/*
 * imu_driver.cpp
 *
 *  Created on: May 19, 2026
 *      Author: sir_l
 */

#include "imu_driver.h"

#include "esp_log.h"
#include <BNO08x.hpp>

static const char *TAG = "imu_driver";

static BNO08x imu; //declares "object"
static bool initialized = false;

bool imu_init(void){
	ESP_LOGI(TAG, "Starting BNO085 SPI init...");
	
	if(!imu.initialize()){
		ESP_LOGE(TAG, "BNO085 init failed. Check wiring");
		return false;
	}
	
	ESP_LOGI(TAG, "BNO085 initialized succesfully.");
	
	//Game rotation vector = fused orientation without magenetometer correction
	// 100000 us = 100 ms = 10 Hz
	imu.rpt.rv_game.enable(100000UL);
	
	//Calibrated gyro report
	imu.rpt.cal_gyro.enable(100000UL);
	
	initialized = true;
	
	return true;
}

bool imu_read(imu_data_t * out){
	
	if(out == NULL ||!initialized){
		return false;
	}
	
	memset(out, 0, sizeof(*out)); //Initializes everything in *out to zero
	out->timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
	
	if(!imu.data_available()){
		out->valid = false;
		return false;
	}
	
	if(imu.rpt.rv_game.has_new_data()){ //Checks if data is available
		bno08x_euler_angle_t euler = imu.rpt.rv_game.get_euler(); //reads euler angles
		
		//Inserts data
		out->roll_deg = euler.x;
		out->pitch_deg = euler.y;
		out->yaw_deg = euler.z;
		
		out->has_orientation = true;
	}
	
	if(imu.rpt.cal_gyro.has_new_data()){
		bno08x_gyro_t gyro = imu.rpt.cal_gyro.get();
		
		out->gyro_x = gyro.x;
		out->gyro_y = gyro.y;
		out->gyro_z = gyro.z;
		
		out->has_gyro = true;
	}
	
	out->valid = out->has_orientation || out->has_gyro;
	
	return out->valid;
}