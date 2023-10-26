/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT rak1901

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>

#include "rak1901.h"

LOG_MODULE_REGISTER(RAK1901, CONFIG_SENSOR_LOG_LEVEL);

static uint8_t rak1901_compute_crc(uint8_t *data)
{
    return crc8(data, 2, RAK1901_CRC_POLY, RAK1901_CRC_INIT, false);
}

static int rak1901_write_command(const struct device *dev, uint32_t cmd)
{
	const struct rak1901_config *cfg = dev->config;
	uint8_t tx_buf[2];

	sys_put_be16(cmd, tx_buf);
	return i2c_write_dt(&cfg->bus, tx_buf, sizeof(tx_buf));
}

/*static int rak1901_sys_reset(const struct device *dev)
{
	const struct rak1901_config *cfg = dev->config;
	uint8_t tx_buf[2] = { RAK1901_CMD_RESET };

	return i2c_write_dt(&cfg->bus, tx_buf, sizeof(tx_buf));
}*/

static int rak1901_read_sample(const struct device *dev,
		uint32_t *t_sample,
		uint32_t *rh_sample)
{
	const struct rak1901_config *cfg = dev->config;
	uint8_t rx_buf[6];
	int rc;

	rc = i2c_read_dt(&cfg->bus, rx_buf, sizeof(rx_buf));
	if (rc < 0) {
		LOG_ERR("Failed to read data from device.");
		return rc;
	}
    
    if (rak1901_compute_crc(rx_buf) != rx_buf[2]) {
        LOG_ERR("Invalid CRC for T.");
        return -EIO;
    }
    if (rak1901_compute_crc(rx_buf+3) != rx_buf[5]) {
        LOG_ERR("Invalid CRC for RH.");
        return -EIO;
    }
	*t_sample = rx_buf[0] << 8 | rx_buf[1];
	*rh_sample = rx_buf[3] << 8 | rx_buf[4];

	return 0;
}

static int rak1901_sample_fetch(const struct device *dev,
			       enum sensor_channel chan)
{
	struct rak1901_data *data = dev->data;
	int rc;

	if (chan != SENSOR_CHAN_ALL &&
		chan != SENSOR_CHAN_AMBIENT_TEMP &&
		chan != SENSOR_CHAN_HUMIDITY) {
		return -ENOTSUP;
	}

	rc = rak1901_write_command(dev, SHTC3_CMD_CSE_TF_NPM);
	if (rc < 0) {
		LOG_ERR("Failed to start measurement.");
		return rc;
	}

	k_sleep(K_MSEC(RAK1901_MEASURE_WAIT_MS));

	rc = rak1901_read_sample(dev, &data->t_sample, &data->rh_sample);
	if (rc < 0) {
		LOG_ERR("Failed to fetch data.");
		return rc;
	}

	return 0;
}

static int rak1901_channel_get(const struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	const struct rak1901_data *data = dev->data;

	/*
	 * See datasheet "Signal Conversion" section
	 * for more details on processing sample data.
	 */
	if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
		float temp;

		temp = (-45 + 175 * ((float) data->t_sample  / 65535));
		sensor_value_from_double(val, temp);
	} else if (chan == SENSOR_CHAN_HUMIDITY) {
		float hum;

		hum = (100 * ((float) data->rh_sample / 65535));
		sensor_value_from_double(val, hum);
	} else {
		return -ENOTSUP;
	}

	return 0;
}

static int rak1901_init(const struct device *dev)
{
	const struct rak1901_config *cfg = dev->config;

	if (!device_is_ready(cfg->bus.bus)) {
		LOG_ERR("Device not ready.");
		return -ENODEV;
	}

/*	k_sleep(K_MSEC(RAK1901_PWRON_WAIT_MS));

	rc = rak1901_sys_reset(dev);
	if (rc < 0) {
		LOG_ERR("Failed to reset the device.");
		return rc;
	}*/
/*
	k_sleep(K_MSEC(RAK1901_RESET_WAIT_MS));

	rc = rak1901_write_command(dev, RAK1901_CMD_INITIAL);
	if (rc < 0) {
		LOG_ERR("Failed to initial the device.");
		return rc;
	}*/

	return 0;
}


static const struct sensor_driver_api rak1901_api = {
	.sample_fetch = rak1901_sample_fetch,
	.channel_get = rak1901_channel_get,
};

#define RAK1901_INIT(n)						\
	static struct rak1901_data rak1901_data_##n;		\
								\
	static const struct rak1901_config rak1901_config_##n = {	\
		.bus = I2C_DT_SPEC_INST_GET(n),			\
	};							\
								\
	SENSOR_DEVICE_DT_INST_DEFINE(n,				\
			      rak1901_init,			\
			      NULL,				\
			      &rak1901_data_##n,			\
			      &rak1901_config_##n,		\
			      POST_KERNEL,			\
			      CONFIG_SENSOR_INIT_PRIORITY,	\
			      &rak1901_api);

DT_INST_FOREACH_STATUS_OKAY(RAK1901_INIT)
