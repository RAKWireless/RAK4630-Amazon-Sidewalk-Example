/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <my_tasks.h>
#include <sm_task.h>
#include <sm_buttons.h>
#include <sm_leds.h>
#include <pal_init.h>
#include <hal/nrf_power.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <sidewalk_version.h>

#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_main, CONFIG_SIDEWALK_LOG_LEVEL);


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

	if ((nrf_power_mainregstatus_get(NRF_POWER) ==
	     NRF_POWER_MAINREGSTATUS_HIGH))
		LOG_INF("Main regulator status: High voltage mode. Voltage supplied on VDDH.");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 1.8V");

	if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos))
		LOG_INF("Output voltage from REG0 regulator stage: 3V");

	rc = application_pal_init();
	if (rc) {
		LOG_INF("Failed to initialize PAL layer.");
		return rc;
	}

	LOG_INF("Sidewalk demo started!");

	sm_task_start();
    my_tasks_init();

	return 0;
}
