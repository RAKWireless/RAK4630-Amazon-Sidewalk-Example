/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT asair_aht20

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>

#include "aht20.h"

LOG_MODULE_REGISTER(AHT20, CONFIG_SENSOR_LOG_LEVEL);

static uint8_t aht20_compute_crc(uint8_t *data)
{
	return crc8(data, 6, AHT20_CRC_POLY, AHT20_CRC_INIT, false);
}

static int aht20_write_command(const struct device *dev, uint32_t cmd)
{
	const struct aht20_config *cfg = dev->config;
	uint8_t tx_buf[3];

	sys_put_be24(cmd, tx_buf);
	return i2c_write_dt(&cfg->bus, tx_buf, sizeof(tx_buf));
}

static int aht20_sys_reset(const struct device *dev)
{
	const struct aht20_config *cfg = dev->config;
	uint8_t tx_buf[1] = { AHT20_CMD_RESET };

	return i2c_write_dt(&cfg->bus, tx_buf, sizeof(tx_buf));
}

static int aht20_read_sample(const struct device *dev,
		uint32_t *t_sample,
		uint32_t *rh_sample)
{
	const struct aht20_config *cfg = dev->config;
	uint8_t rx_buf[7];
	int rc;

	rc = i2c_read_dt(&cfg->bus, rx_buf, sizeof(rx_buf));
	if (rc < 0) {
		LOG_ERR("Failed to read data from device.");
		return rc;
	}

	if (aht20_compute_crc(rx_buf) != rx_buf[6]) {
		LOG_ERR("Invalid CRC for T. and RH.");
		return -EIO;
	}

	*t_sample = ((rx_buf[3] & 0x0F) << 16) | (rx_buf[4] << 8) | rx_buf[5];
	*rh_sample = ((rx_buf[1] << 16) | (rx_buf[2] << 8) | rx_buf[3]) >> 4;

	return 0;
}

static int aht20_sample_fetch(const struct device *dev,
			       enum sensor_channel chan)
{
	struct aht20_data *data = dev->data;
	int rc;

	if (chan != SENSOR_CHAN_ALL &&
		chan != SENSOR_CHAN_AMBIENT_TEMP &&
		chan != SENSOR_CHAN_HUMIDITY) {
		return -ENOTSUP;
	}

	rc = aht20_write_command(dev, AHT20_CMD_MEASURE);
	if (rc < 0) {
		LOG_ERR("Failed to start measurement.");
		return rc;
	}

	k_sleep(K_MSEC(AHT20_MEASURE_WAIT_MS));

	rc = aht20_read_sample(dev, &data->t_sample, &data->rh_sample);
	if (rc < 0) {
		LOG_ERR("Failed to fetch data.");
		return rc;
	}

	return 0;
}

static int aht20_channel_get(const struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	const struct aht20_data *data = dev->data;

	/*
	 * See datasheet "Signal Conversion" section
	 * for more details on processing sample data.
	 */
	if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		float temp;

		temp = (data->t_sample * 200) / 1048576.0f - 50.0f;
		sensor_value_from_double(val, temp);
	} else if (chan == SENSOR_CHAN_HUMIDITY) {
		float hum;

		hum = (data->rh_sample * 100) / 1048576.0f;
		sensor_value_from_double(val, hum);
	} else {
		return -ENOTSUP;
	}

	return 0;
}

static int aht20_init(const struct device *dev)
{
	const struct aht20_config *cfg = dev->config;
	int rc = 0;

	if (!device_is_ready(cfg->bus.bus)) {
		LOG_ERR("Device not ready.");
		return -ENODEV;
	}

	k_sleep(K_MSEC(AHT20_PWRON_WAIT_MS));

	rc = aht20_sys_reset(dev);
	if (rc < 0) {
		LOG_ERR("Failed to reset the device.");
		return rc;
	}

	k_sleep(K_MSEC(AHT20_RESET_WAIT_MS));

	rc = aht20_write_command(dev, AHT20_CMD_INITIAL);
	if (rc < 0) {
		LOG_ERR("Failed to initial the device.");
		return rc;
	}

	return 0;
}


static const struct sensor_driver_api aht20_api = {
	.sample_fetch = aht20_sample_fetch,
	.channel_get = aht20_channel_get,
};

#define AHT20_INIT(n)						\
	static struct aht20_data aht20_data_##n;		\
								\
	static const struct aht20_config aht20_config_##n = {	\
		.bus = I2C_DT_SPEC_INST_GET(n),			\
	};							\
								\
	SENSOR_DEVICE_DT_INST_DEFINE(n,				\
			      aht20_init,			\
			      NULL,				\
			      &aht20_data_##n,			\
			      &aht20_config_##n,		\
			      POST_KERNEL,			\
			      CONFIG_SENSOR_INIT_PRIORITY,	\
			      &aht20_api);

DT_INST_FOREACH_STATUS_OKAY(AHT20_INIT)
