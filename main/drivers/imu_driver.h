#pragma once

#include <stdbool.h>
#include "payload_data.h"

#ifdef __cplusplus
extern "C"{
#endif

bool imu_init(void);
bool imu_read(imu_data_t * out);

#ifdef __cplusplus
}
#endif