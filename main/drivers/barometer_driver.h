/*
 * barometer_driver.h
 *
 *  Created on: May 18, 2026
 *      Author: sir_l
 */
#pragma once

#ifndef MAIN_DRIVERS_BAROMETER_DRIVER_H_
#define MAIN_DRIVERS_BAROMETER_DRIVER_H_
#include <stdbool.h>
#include "payload_data.h"

static float pressure_to_altitude_m(float pressure_pa);
bool barometer_init(void);
bool barometer_read(barometer_data_t *out);


#endif /* MAIN_DRIVERS_BAROMETER_DRIVER_H_ */
