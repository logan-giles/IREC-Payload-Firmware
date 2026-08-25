/*
 * payload_data.h
 *
 *  Created on: May 18, 2026
 *      Author: sir_l
 */
#pragma once

#ifndef MAIN_COMMON_PAYLOAD_DATA_H_
#define MAIN_COMMON_PAYLOAD_DATA_H_



#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct{
	uint32_t timestamp_ms;
	float pressure_pa;
	float temperature_c;
	float altitude_m;
	
	bool valid;
} barometer_data_t;

typedef struct {
	uint32_t timestamp_ms;
	
	float roll_deg;
	float pitch_deg;
	float yaw_deg;
	
	float gyro_x;
	float gyro_z;
	float gyro_y;
	
	bool has_orientation;
	bool has_gyro;
	bool valid;
} imu_data_t;

typedef struct{
	barometer_data_t baro_data;
	imu_data_t imu_data;
} sensor_data_t;

typedef struct {
	bool imu_ok;
	bool barometer_ok;
	bool storage_ok;
	uint32_t imu_read_fail_count;
	uint32_t barometer_read_fail_count;
	uint32_t dropped_packet_count;
	
	uint32_t sensor_task_loops;
	uint32_t logger_task_loops;
	uint32_t health_task_loops;
} system_status_t;

#endif /* MAIN_COMMON_PAYLOAD_DATA_H_ */


