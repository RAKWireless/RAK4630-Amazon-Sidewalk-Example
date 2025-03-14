/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <app.h>
#include <sidewalk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <sidewalk_version.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);

#define NOTIFY_TIMER_DURATION_MS (500)
#define APP_RX_PAYLOAD_MAX_SIZE 255


static void configure_uicr(void)
{
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) !=
        (UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        LOG_INF("REGOUT0 is updated!");

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}

/*static int init_leds()
{
	int err = sm_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}
	return err;
}*/

int main(void)
{
    configure_uicr();
	int rc;
	if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
		rc = usb_enable(NULL);
		if (rc) {
			LOG_ERR("Failed to enable USB");
		}
	}

	PRINT_SIDEWALK_VERSION();

/*	if ((nrf_power_mainregstatus_get(NRF_POWER) ==
	     NRF_POWER_MAINREGSTATUS_HIGH))
		LOG_INF("Main regulator status: High voltage mode. Voltage supplied on VDDH.");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 1.8V");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 3V");*/

/*	rc = init_leds();
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

    my_tasks_init();
	sm_task_start();*/
    app_start();

	return 0;
}
