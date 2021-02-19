/*
 * (C) 2005-2020 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek"). You may only use, reproduce, modify, or
 * distribute (as applicable) MediaTek Software if you have agreed to and been
 * bound by this Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO
 * LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED
 * HEREUNDER WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * RECEIVER TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH
 * MEDIATEK SOFTWARE AT ISSUE.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
#include "mt3620.h"

#include "os_hal_uart.h"
#include "os_hal_pwm.h"

/****************************************************************************/
/* Configurations */
/****************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

/** -------------------------------------------------------
 * PWM Group        , PWM Channel , PWM Bitmap  : GPIO
 * -------------------------------------------------------
 *  OS_HAL_PWM_GROUP0, PWM_CHANNEL0, OS_HAL_PWM_0: GPIO0
 *  OS_HAL_PWM_GROUP0, PWM_CHANNEL1, OS_HAL_PWM_1: GPIO1
 *  OS_HAL_PWM_GROUP0, PWM_CHANNEL2, OS_HAL_PWM_2: GPIO2
 *  OS_HAL_PWM_GROUP0, PWM_CHANNEL3, OS_HAL_PWM_3: GPIO3
 *  OS_HAL_PWM_GROUP1, PWM_CHANNEL0, OS_HAL_PWM_0: GPIO4
 *  OS_HAL_PWM_GROUP1, PWM_CHANNEL1, OS_HAL_PWM_1: GPIO5
 *  OS_HAL_PWM_GROUP1, PWM_CHANNEL2, OS_HAL_PWM_2: GPIO6
 *  OS_HAL_PWM_GROUP1, PWM_CHANNEL3, OS_HAL_PWM_3: GPIO7
 *  OS_HAL_PWM_GROUP2, PWM_CHANNEL0, OS_HAL_PWM_0: GPIO8 (LED Red)
 *  OS_HAL_PWM_GROUP2, PWM_CHANNEL1, OS_HAL_PWM_1: GPIO9 (LED Green)
 *  OS_HAL_PWM_GROUP2, PWM_CHANNEL2, OS_HAL_PWM_2: GPIO10 (LED Blue)
 *  OS_HAL_PWM_GROUP2, PWM_CHANNEL3, OS_HAL_PWM_3: GPIO11
 * -------------------------------------------------------
 */

static const uint8_t pwm_group_led_red = OS_HAL_PWM_GROUP2;
static const uint8_t pwm_channel_led_red = PWM_CHANNEL0;
static const uint8_t pwm_bitmap_led_red = OS_HAL_PWM_0;
static const uint32_t pwm_frequency_led_red = 2000000;
static uint32_t pwm_duty_led_red = 333;

static const uint8_t pwm_group_led_green = OS_HAL_PWM_GROUP2;
static const uint8_t pwm_channel_led_green = PWM_CHANNEL1;
static const uint8_t pwm_bitmap_led_green = OS_HAL_PWM_1;
static const uint32_t pwm_frequency_led_green = 2000000;
static uint32_t pwm_duty_led_green = 666;

static const uint8_t pwm_group_led_blue = OS_HAL_PWM_GROUP2;
static const uint8_t pwm_channel_led_blue = PWM_CHANNEL2;
static const uint8_t pwm_bitmap_led_blue = OS_HAL_PWM_2;
static const uint32_t pwm_frequency_led_blue = 2000000;
static uint32_t pwm_duty_led_blue = 999;

#define GLOBAL_KICK 0
#define IO_CTRL 0
#define POLARITY_SET 1

#define APP_STACK_SIZE_BYTES		(1024 / 4)

/****************************************************************************/
/* Applicaiton Hooks */
/****************************************************************************/
/* Hook for "stack over flow". */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("%s: %s\n", __func__, pcTaskName);
}

/* Hook for "memory allocation failed". */
void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __func__);
}
/* Hook for "printf". */
void _putchar(char character)
{
	mtk_os_hal_uart_put_char(uart_port_num, character);
	if (character == '\n')
		mtk_os_hal_uart_put_char(uart_port_num, '\r');
}

/****************************************************************************/
/* Functions */
/****************************************************************************/
void pwm_task(void *pParameters)
{
	int ret = 0;

	printf("PWM Task Start\n");

	/* Init PWM */
	ret |= mtk_os_hal_pwm_ctlr_init(pwm_group_led_red,
					pwm_bitmap_led_red |
					pwm_bitmap_led_green |
					pwm_bitmap_led_blue);
	if (ret) {
		printf("mtk_os_hal_pwm_ctlr_init failed\n");
		goto err_exit;
	}

	/* Configure PWM */
	ret |= mtk_os_hal_pwm_feature_enable(pwm_group_led_red,
					     pwm_channel_led_red,
					     GLOBAL_KICK,
					     IO_CTRL,
					     POLARITY_SET);
	ret |= mtk_os_hal_pwm_feature_enable(pwm_group_led_green,
					     pwm_channel_led_green,
					     GLOBAL_KICK,
					     IO_CTRL,
					     POLARITY_SET);
	ret |= mtk_os_hal_pwm_feature_enable(pwm_group_led_blue,
					     pwm_channel_led_blue,
					     GLOBAL_KICK,
					     IO_CTRL,
					     POLARITY_SET);
	if (ret) {
		printf("mtk_os_hal_pwm_feature_enable failed\n");
		goto err_exit;
	}

	/* Follow the below steps to start the PWM for one channel */
	/* Step1: Call mtk_os_hal_pwm_config_freq_duty_normal() for one channel */
	/* Step2: Call mtk_os_hal_pwm_start_normal() to start right after step1 */

	/* Set PWM Frequency & Duty, and start PWM for LED Red */
	ret |= mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_red,
						      pwm_channel_led_red,
						      pwm_frequency_led_red,
						      pwm_duty_led_red);

	ret |= mtk_os_hal_pwm_start_normal(pwm_group_led_red,
		pwm_channel_led_red);

	if (ret) {
		printf("PWM config and start for LED Red failed\n");
		goto err_exit;
	}

	/* Set PWM Frequency & Duty, and start PWM for LED Green */

	ret |= mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_green,
						      pwm_channel_led_green,
						      pwm_frequency_led_green,
						      pwm_duty_led_green);
	ret |= mtk_os_hal_pwm_start_normal(pwm_group_led_green,
		pwm_channel_led_green);

	if (ret) {
		printf("PWM config and start for LED Green failed\n");
		goto err_exit;
	}

	/* Set PWM Frequency & Duty, and start PWM for LED Blue */
	ret |= mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_blue,
						      pwm_channel_led_blue,
						      pwm_frequency_led_blue,
						      pwm_duty_led_blue);

	ret |= mtk_os_hal_pwm_start_normal(pwm_group_led_blue,
					   pwm_channel_led_blue);
	if (ret) {
		printf("PWM config and start for LED Blue failed\n");
		goto err_exit;
	}

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));

		pwm_duty_led_red += 1;
		if (pwm_duty_led_red > 1000)
			pwm_duty_led_red = 0;

		pwm_duty_led_green += 1;
		if (pwm_duty_led_green > 1000)
			pwm_duty_led_green = 0;

		pwm_duty_led_blue += 1;
		if (pwm_duty_led_blue > 1000)
			pwm_duty_led_blue = 0;

		/* Change PWM Duty */
		ret |= mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_red,
							pwm_channel_led_red,
							pwm_frequency_led_red,
							pwm_duty_led_red);
		ret |=
		  mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_green,
							pwm_channel_led_green,
							pwm_frequency_led_green,
							pwm_duty_led_green);
		ret |=
		  mtk_os_hal_pwm_config_freq_duty_normal(pwm_group_led_blue,
							pwm_channel_led_blue,
							pwm_frequency_led_blue,
							pwm_duty_led_blue);
		if (ret) {
			printf("%s failed\n", __func__);
			goto err_exit;
		}
	}

	/* Stop PWM */
	ret |= mtk_os_hal_pwm_stop_normal(pwm_group_led_red,
					  pwm_channel_led_red);
	ret |= mtk_os_hal_pwm_stop_normal(pwm_group_led_green,
					  pwm_channel_led_green);
	ret |= mtk_os_hal_pwm_stop_normal(pwm_group_led_blue,
					  pwm_channel_led_blue);
	if (ret) {
		printf("mtk_os_hal_pwm_stop_normal failed\n");
		goto err_exit;
	}

	/* Deinit PWM */
	ret |= mtk_os_hal_pwm_ctlr_deinit(pwm_group_led_red,
					  pwm_bitmap_led_red |
					  pwm_bitmap_led_green |
					  pwm_bitmap_led_blue);
	if (ret) {
		printf("mtk_os_hal_pwm_ctlr_deinit failed\n");
		goto err_exit;
	}

err_exit:
	printf("PWM Task Finish\n");
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS PWM Demo\n");

	/* Create PWM Task */
	xTaskCreate(pwm_task, "PWM Task", APP_STACK_SIZE_BYTES, NULL, 5, NULL);
	vTaskStartScheduler();

	for (;;)
		__asm__("wfi");
}

