/*
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_AHT20_AHT20_H_
#define ZEPHYR_DRIVERS_SENSOR_AHT20_AHT20_H_

#include <zephyr/device.h>

#define AHT20_CMD_RESET		0xBA
#define AHT20_CMD_INITIAL	0xBE0800
#define AHT20_CMD_MEASURE	0xAC3300

#define AHT20_RESET_WAIT_MS	20
#define AHT20_PWRON_WAIT_MS	40
#define AHT20_MEASURE_WAIT_MS	80

/*
 * CRC parameters were taken from the
 * "Sensor reading process" section of the datasheet.
 */
#define AHT20_CRC_POLY		0x31
#define AHT20_CRC_INIT		0xFF

struct aht20_config {
	struct i2c_dt_spec bus;
};

struct aht20_data {
	uint32_t t_sample;
	uint32_t rh_sample;
};

#endif /* ZEPHYR_DRIVERS_SENSOR_AHT20_AHT20_H_ */
