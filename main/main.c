#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "imu_driver.h"
#include "barometer_driver.h"
#include "storage_driver.h"
#include "payload_data.h"

#define RUN_STORAGE_TEST 1

/* Note: USE ESP_LOGI for informational prints
		 USE ESP_LOGW for stuff that isn't exactly a fatal error but something that should be notified
		 USE ESP_LOGE for things like fatal or dramatic errors*/

//Variables:
sensor_data_t sensor_data = {0};
system_status_t health_data = {0};

//Task Handles
TaskHandle_t sensorTaskHandle;
TaskHandle_t loggerTaskHandle;
TaskHandle_t healthTaskHandle;

//Queue
QueueHandle_t sensorData = NULL;



//Tags
static const char * baroTAG = "Barometer";
static const char * imuTAG = "IMU";
static const char * logTAG = "Logger";
static const char * healthTAG = "SYSTEM ALIVE";
static const char * mainTAG = "Main";

//Task Functions
void sensorTask(void *pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = pdMS_TO_TICKS(100); //Change pdMS_TO_TICKS paramter to change task occurence frequency
	sensor_data_t sensorPacket;
	memset(&sensorPacket, 0, sizeof(sensor_data_t));
	
	/*Initialize Baro and IMU REDUNDANCY VERSION REV 1 FOR FLIGHT USE
	for(int i = 0; i < 3; i++){
		if(!barometer_init()){
			health_data.barometer_ok = false;
			continue;
		}
		health_data.barometer_ok = true;
		break;
	}
	
	for(int i = 0; i < 3; i++){
		if(!imu_init()){
			health_data.imu_ok = false;
			continue;
		}
		health_data.imu_ok = true;
		break;
	}
	*/
	
	
	
	while(1){
		if(health_data.imu_ok){
			if(!imu_read(&(sensorPacket.imu_data))){
				health_data.imu_read_fail_count++;
			}
		}
		
		if(health_data.barometer_ok){
			if(!barometer_read(&(sensorPacket.baro_data))){
				health_data.barometer_read_fail_count++;
			}
		}
		
		//Sends to queue
		if((xQueueSend(sensorData, &sensorPacket, 0) != pdTRUE)){ //Counts packetloss and sends to queue
			health_data.dropped_packet_count++;
		}
		
		health_data.sensor_task_loops++;
		
		//Delay/Blocking according to modifiable frequency
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void loggerTask(void *pvParameters){
	sensor_data_t serialBuffer;
	char line[256];
	
	while(1){
		if(xQueueReceive(sensorData, &serialBuffer, portMAX_DELAY) == pdTRUE){
		
			/*For now we will print data even if its invalid for the sake of 
			debugging, later we will likely ignore invalid data*/
			//Barometer data print
			ESP_LOGI(logTAG,
			"| BARO |\nTimestamp: %lu | Pressure(pa): %.2f | Temp(C): %.2f | Altitude(m): %.2f", 
			serialBuffer.baro_data.timestamp_ms, 
			serialBuffer.baro_data.pressure_pa,
			serialBuffer.baro_data.temperature_c, 
			serialBuffer.baro_data.altitude_m);
		
			//IMU data print
			ESP_LOGI(logTAG, 
			"| IMU |\nTimestamp: %lu | Pitch: %.2f | Roll: %.2f | Yaw: %.2f | "
			"X-Velocity(rad/s): %.2f | Y-Velocity(rad/s): %.2f | Z-Velocity(rad/s): %.2f",
			serialBuffer.imu_data.timestamp_ms, 
			serialBuffer.imu_data.pitch_deg,
			serialBuffer.imu_data.roll_deg, 
			serialBuffer.imu_data.yaw_deg, 
			serialBuffer.imu_data.gyro_x, 
			serialBuffer.imu_data.gyro_y, 
			serialBuffer.imu_data.gyro_z);
			
			/*MicroSD writing, right now we will write the health and sensor data into the same CSV file 
			, in the future the data will be separated into two separate files and written into at different intervals*/
			snprintf(line, sizeof(line),
				"%lu, %d, %.3f, %.3f, %.3f, %.6f, %.6f, %.6f, %d, %.2f, %.2f, %.2f, %lu, %lu, %lu",
				(unsigned long)serialBuffer.imu_data.timestamp_ms,
				
				serialBuffer.imu_data.valid,
				serialBuffer.imu_data.roll_deg,
				serialBuffer.imu_data.pitch_deg,
				serialBuffer.imu_data.yaw_deg,
				serialBuffer.imu_data.gyro_x,
				serialBuffer.imu_data.gyro_y,
				serialBuffer.imu_data.gyro_z,
				
				serialBuffer.baro_data.valid,
				serialBuffer.baro_data.pressure_pa,
				serialBuffer.baro_data.temperature_c,
				serialBuffer.baro_data.altitude_m,
				
				(unsigned long)health_data.dropped_packet_count,
				(unsigned long)health_data.imu_read_fail_count,
				(unsigned long)health_data.barometer_read_fail_count);
			
			storage_write(line);
			
			health_data.logger_task_loops++;
		}
	}
}

void healthTask(void *pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = pdMS_TO_TICKS(1000); //Change pdMS_TO_TICKS paramter to change task occurence frequency
	
	while(1){
		ESP_LOGI(healthTAG,
		"baro_ok = %d | imu_ok = %d | imu_read_fail = %lu | baro_read_fail = %lu | Dropped_Packets: %lu | Health_loops: %lu | Sensor_loops: %lu",
		health_data.barometer_ok,
		health_data.imu_ok,
		health_data.imu_read_fail_count,
		health_data.barometer_read_fail_count,
		health_data.dropped_packet_count,
		health_data.health_task_loops,
		health_data.sensor_task_loops
		);
		
		health_data.health_task_loops++;
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

//Run Payload tasks function
static void run_payload_tasks(void){
	ESP_LOGI(mainTAG, "Payload firmware starting...");
	
	//Queue
	sensorData = xQueueCreate(32, sizeof(sensor_data_t));
	
	if(sensorData == NULL){
		ESP_LOGI(mainTAG, "Queue creation failed...");
		return;
	}
	
	//MicroSD card init
	if(!storage_init()){
		ESP_LOGE(mainTAG, "Storage initialization failed...");
		health_data.storage_ok = false;
		return;
	} else{
		health_data.storage_ok = true;
	}
	
	//Log_file CSV header row write
	if(!storage_write("timestamp_ms, imu_valid, roll_deg, pitch_deg, yaw_deg, gyro_x, gyro_y, gyro_z, baro_valid, pressure_pa, temp_c, altitude_m, dropped_packets, imu_read_fail_count, baro_read_fail_count")){
		ESP_LOGE(mainTAG, "CSV Header Row writing failed...");
		return;
	}
	
	//imu and barometer initialization
	if(!imu_init()){
		ESP_LOGE(imuTAG, "IMU initialization failed...");
		health_data.imu_ok = false;
		vTaskDelete(NULL);
	} else {
		health_data.imu_ok = true;
	}
	
	if(!barometer_init()){
		ESP_LOGE(baroTAG, "Barometer initialization failed...");
		health_data.barometer_ok = false;
		vTaskDelete(NULL);
	} else {
		health_data.barometer_ok = true;
	}
	
	/*Initialize Baro, IMU, and MicroSD Card REDUNDANCY VERSION REV 1 FOR FLIGHT USE
	for(int i = 0; i < 3; i++){
		if(!barometer_init()){
			health_data.barometer_ok = false;
			continue;
		}
		health_data.barometer_ok = true;
		break;
	}
	
	for(int i = 0; i < 3; i++){
		if(!imu_init()){
			health_data.imu_ok = false;
			continue;
		}
		health_data.imu_ok = true;
		break;
	}
	
	for(int i = 0; i < 3; i++){
		if(!storage_init()){
			health_data.storage_ok = false;
			continue;
		}
		health_data.storage_ok = true;
		break;
		}
	*/

	
	
	//Task Creation
	xTaskCreate(sensorTask, "Sensor", 4800, NULL, 5, &sensorTaskHandle);
	xTaskCreate(loggerTask, "logger", 4800, NULL, 4, &loggerTaskHandle);
	xTaskCreate(healthTask, "health", 3072, NULL, 1, &healthTaskHandle);
	
	ESP_LOGI(mainTAG, "Queue and Tasks created...");
}

//Storage Test
static void storage_test(void){
	ESP_LOGI(mainTAG, "Starting SD storage test...");

    if (!storage_init()) {
        ESP_LOGE(mainTAG, "storage_init failed");
        return;
    }

    ESP_LOGI(mainTAG, "storage_init success");

    if (!storage_write("time_ms,pitch,roll,yaw,pressure,temp")) {
        ESP_LOGE(mainTAG, "Failed to write CSV header");
    }

    if (!storage_write("100,1.23,4.56,7.89,101325.00,22.50")) {
        ESP_LOGE(mainTAG, "Failed to write test row");
    }

    if (!storage_flush()) {
        ESP_LOGE(mainTAG, "storage_flush failed");
    }

    char buffer[256];

    if (storage_read(buffer, sizeof(buffer))) {
        ESP_LOGI(mainTAG, "Read first line: %s", buffer);
    } else {
        ESP_LOGE(mainTAG, "Failed to read first line");
    }

    storage_deinit();

    ESP_LOGI(mainTAG, "SD storage test finished");
}

void app_main(void){
	#if RUN_STORAGE_TEST
		storage_test();
	#else
		run_payload_tasks();
	#endif
}






