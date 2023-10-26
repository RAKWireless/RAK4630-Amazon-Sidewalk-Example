/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sm_task.h>
#include <sm_buttons.h>
#include <sm_leds.h>
#include <pal_init.h>
#include <hal/nrf_power.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <sidewalk_version.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);

static int init_leds_and_buttons()
{
	int err = sm_buttons_init();

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = sm_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}
	return err;
}

int main(void)
{
	int rc;
	if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
		rc = usb_enable(NULL);
		if (rc) {
			LOG_ERR("Failed to enable USB");
		}
	}

	PRINT_SIDEWALK_VERSION();

	if ((nrf_power_mainregstatus_get(NRF_POWER) ==
	     NRF_POWER_MAINREGSTATUS_HIGH))
		LOG_INF("Main regulator status: High voltage mode. Voltage supplied on VDDH.");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 1.8V");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 3V");

	rc = init_leds_and_buttons();
	if (rc) {
		LOG_INF("Buttons or LEDs faild to initialize.");
		return rc;
	}

	rc = application_pal_init();
	if (rc) {
		LOG_INF("Failed to initialize PAL layer.");
		return rc;
	}

	LOG_INF("Sidewalk demo started!");

	sm_task_start();

	return 0;
}
