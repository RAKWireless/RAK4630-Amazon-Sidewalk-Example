#include "rak_sidewalk.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rak_sidewalk, CONFIG_SIDEWALK_LOG_LEVEL);

sidewalk_payload_t rak_sidewalk_get_payload(void)
{
    const struct device *dev = DEVICE_DT_GET_ONE(rak1901);
    struct sensor_value temp_sensor_value, humidity_sensor_value;
    float temp=0.0f, hum=0.0f;
    sidewalk_payload_t payload={0};
    if (!device_is_ready (dev)) {
        LOG_ERR("sensor: device not ready");
        return payload;
    }
    sensor_sample_fetch (dev);

    sensor_channel_get (dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_sensor_value);
    sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity_sensor_value);

    temp = (float) sensor_value_to_double(&temp_sensor_value);
    hum = (float) sensor_value_to_double (&humidity_sensor_value);
    payload.payload_type = 0x01;
    payload.temp = temp;
    payload.hum = hum;
    return payload;
}
