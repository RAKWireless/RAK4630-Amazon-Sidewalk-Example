/*
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_RAK1901_H_
#define ZEPHYR_DRIVERS_SENSOR_RAK1901_H_

#include <zephyr/device.h>

#define RAK1901_CMD_RESET	0x805D
#define RAK1901_CMD_WAKE    0x3517
#define RAK1901_CMD_SLEEP   0xB098
#define RAK1901_CMD_READ_ID	0xEFC8

#define RAK1901_RESET_WAIT_MS	20
#define RAK1901_PWRON_WAIT_MS	40
#define RAK1901_MEASURE_WAIT_MS	80

#define SHTC3_CMD_CSE_RHF_NPM 0x5C24 // Clock stretching, RH first,Normal power mode
#define SHTC3_CMD_CSE_RHF_LPM 0x44DE // Clock stretching, RH first, Low power mode

#define SHTC3_CMD_CSE_TF_NPM 0x7CA2 // Clock stretching, T first,Normal power mode
#define SHTC3_CMD_CSE_TF_LPM 0x6458 // Clock stretching, T first, Low power mode

#define SHTC3_CMD_CSD_RHF_NPM 0x58E0 // Polling, RH first, Normal power mode
#define SHTC3_CMD_CSD_RHF_LPM 0x401A // Polling, RH first, Low power mode

#define SHTC3_CMD_CSD_TF_NPM 0x7866 // Polling, T first, Normal power mode
#define SHTC3_CMD_CSD_TF_LPM 0x609C // Polling, T first, Low power mode

/*
 * CRC parameters were taken from the
 * "Sensor reading process" section of the datasheet.
 */
#define RAK1901_CRC_POLY		0x31
#define RAK1901_CRC_INIT		0xFF

struct rak1901_config {
	struct i2c_dt_spec bus;
};

struct  rak1901_data {
	uint32_t t_sample;
	uint32_t rh_sample;
};

#endif /* ZEPHYR_DRIVERS_SENSOR_AHT20_AHT20_H_ */
