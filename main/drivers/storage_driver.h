/*
 * storage_driver.h
 *
 *  Created on: May 21, 2026
 *      Author: sir_l
 */

#ifndef MAIN_DRIVERS_STORAGE_DRIVER_H_
#define MAIN_DRIVERS_STORAGE_DRIVER_H_
#include <stdbool.h>
#include <stddef.h>

bool storage_init(void); //Mount microSD and open log file
bool storage_write(const char * line); //Write CSV rows
bool storage_read(char *buffer, size_t buffer_size); //Test reading back data
bool storage_flush(void); //Force buffered data to card
bool storage_deinit(void); // Close file and unmount card



#endif /* MAIN_DRIVERS_STORAGE_DRIVER_H_ */
