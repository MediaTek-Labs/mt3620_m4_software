/*
 * (C) 2005-2019 MediaTek Inc. All rights reserved.
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

#ifndef __MHAL_MBOX_H__
#define __MHAL_MBOX_H__

#include <mhal_osai.h>

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup MBOX
 * @{
 * This section introduces the Mailbox (MBOX) APIs
 * including terms and acronyms, supported features, software architecture,
 * details on how to use this driver, enums, structures and functions.
 *
 * @section MHAL_MBOX_Terms_Chapter Terms and Acronyms
 *
 * |Terms                   |Details                             |
 * |------------------------------|--|
 * |\b FIFO                       | First In, First Out.|
 * |\b MBOX                       | Mailbox.|
 *
 * @section MHAL_MBOX_Features_Chapter Supported Features
 *
 * The MBOX supports
 * FIFO/FIFO interrupt/Software interrupt/Semaphore.\n
 * For more details, please refer to MBOX datasheet.
 *
 * @}
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup MBOX
 * @{
 * @section MHAL_MBOX_Driver_Usage_Chapter How to use this driver
 *
 * - \b SW \b Architecture: \n
 * Refer to @ref MHAL_Overview_2_Chapter for details of the SW architecture.
 *
 *  This section describes the definition of APIs and provides
 *  an example of how to use these APIs to develop
 *  an OS-related MBOX driver on FreeRTOS.\n
 *  This section also provides an example of the sequence of APIs
 *  that should be used by the user application.
 *
 * - \b The \b OS-HAL \b FreeRTOS \b driver\n
 * \b sample \b code \b is \b as \b follows:\n
 *  - Sample code (FreeRTOS does not have MBOX framework,
 * so this sample code provides APIs to user application):
 *    @code
 *
 *	#define MTK_MBOX_DEBUG_ENABLE 0
 *
 *	#define OS_MBOX_ERROR(fmt, arg...)	printf(\
 *					"[os_mbox][error] %s(L:%d) "fmt,\
 *						__func__, __LINE__, ##arg)
 *	#if MTK_MBOX_DEBUG_ENABLE
 *	#define OS_MBOX_DEBUG(fmt, arg...)	printf(\
 *					"[os_mbox][debug] %s(L:%d) "fmt,\
 *						__func__, __LINE__, ##arg)
 *	#else
 *	#define OS_MBOX_DEBUG(fmt, arg...)
 *	#endif
 *
 *
 *	#define MBOX_M4_VECTOR_FIFO0			CM4_IRQ_M42A7N_FIFO
 *	#define MBOX_M4_VECTOR_FIFO1			CM4_IRQ_M42M4_FIFO
 *
 *	#define MBOX_M4_VECTOR_CH0_SW			CM4_IRQ_A7N2M4_SW
 *	#define MBOX_M4_VECTOR_CH1_SW			CM4_IRQ_M42M4_SW
 *
 *	#define MBOX_M4_VECTOR_CH0_WR			CM4_IRQ_A7N2M4_WR
 *	#define MBOX_M4_VECTOR_CH1_WR			CM4_IRQ_M42M4_WR
 *
 *	#define MBOX_M4_VECTOR_CH0_RD			CM4_IRQ_M42A7N_RD
 *	#define MBOX_M4_VECTOR_CH1_RD			CM4_IRQ_M42M4_RD
 *
 *	#define MBOX_M4_VECTOR_CH0_NE			CM4_IRQ_A7N2M4_NE
 *	#define MBOX_M4_VECTOR_CH1_NE			CM4_IRQ_M42M4_NE
 *
 *	#define MBOX_M4_VECTOR_CH0_NF			CM4_IRQ_M42A7N_NF
 *	#define MBOX_M4_VECTOR_CH1_NF			CM4_IRQ_M42M4_NF
 *
 *
 *	#define CH0_MBOX_BASE			0x21050000
 *	#define CH1_MBOX_BASE			0x21060000
 *
 *	static unsigned long mbox_base_addr[OS_HAL_MBOX_CH_MAX] = {
 *		CH0_MBOX_BASE,
 *		CH1_MBOX_BASE,
 *	};
 *
 *	static u32 wr_vectors[OS_HAL_MBOX_CH_MAX] = {MBOX_M4_VECTOR_CH0_WR,
 *							MBOX_M4_VECTOR_CH1_WR};
 *	static u32 rd_vectors[OS_HAL_MBOX_CH_MAX] = {MBOX_M4_VECTOR_CH0_RD,
 *							MBOX_M4_VECTOR_CH1_RD};
 *	static u32 ne_vectors[OS_HAL_MBOX_CH_MAX] = {MBOX_M4_VECTOR_CH0_NE,
 *							MBOX_M4_VECTOR_CH1_NE};
 *	static u32 nf_vectors[OS_HAL_MBOX_CH_MAX] = {MBOX_M4_VECTOR_CH0_NF,
 *							MBOX_M4_VECTOR_CH1_NF};
 *	static u32 sw_vectors[OS_HAL_MBOX_CH_MAX] = {MBOX_M4_VECTOR_CH0_SW,
 *							MBOX_M4_VECTOR_CH1_SW};
 *
 *	struct os_hal_mbox_channel {
 *		SemaphoreHandle_t sem_fifo_read;
 *		SemaphoreHandle_t sem_fifo_write;
 *
 *		u32 swint_en;
 *		mtk_os_hal_mbox_cb swint_cb;
 *		SemaphoreHandle_t sem_sw_int;
 *
 *		mtk_os_hal_mbox_cb fifo_cb;
 *		SemaphoreHandle_t sem_fifo_int;
 *
 *		struct mbox_fifo_event event;
 *		struct mbox_fifo_event mask;
 *		struct mbox_swint_info swint;
 *
 *		void __iomem *base;
 *		int enable;
 *	};
 *
 *	struct os_hal_mbox_device {
 *		struct os_hal_mbox_channel	channels[OS_HAL_MBOX_CH_MAX];
 *	};
 *
 *	static struct os_hal_mbox_device mbox;
 *	static struct os_hal_mbox_device *mbox_dev = &mbox;
 *
 *	static struct os_hal_mbox_channel *_mtk_os_hal_mbox_get_channel(
 *					mbox_channel_t channel)
 *	{
 *		if (channel >= OS_HAL_MBOX_CH_MAX
 *			|| channel < OS_HAL_MBOX_CH0) {
 *			OS_MBOX_ERROR(
 *				"channel:%d out of range, should be:0~%d\n",
 *				channel, OS_HAL_MBOX_CH_MAX-1);
 *			return NULL;
 *		}
 *		if (!mbox_dev->channels[channel].enable) {
 *			OS_MBOX_ERROR("channel %d is not open\n", channel);
 *			return NULL;
 *		}
 *
 *		return &mbox_dev->channels[channel];
 *	}
 *
 *	static void _mtk_os_hal_mbox_sw_int_irq_handler(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		struct mtk_os_hal_mbox_cb_data cb_data;
 *		void __iomem *base;
 *		u32 status;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return;
 *		}
 *		base = ch->base;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOGET_SWINT_EN, &(ch->swint_en));
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOGET_SWINT_STS, &status);
 *		ch->swint.channel = channel;
 *		ch->swint.swint_sts = ch->swint_en & status;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_SWINT, &status);
 *
 *		cb_data.swint.channel = ch->swint.channel;
 *		cb_data.swint.swint_sts = ch->swint.swint_sts;
 *
 *		if (cb_data.swint.swint_sts) {
 *			xSemaphoreTakeFromISR(ch->sem_sw_int, NULL);
 *			if (ch->swint_cb != NULL)
 *				ch->swint_cb(&cb_data);
 *			xSemaphoreGiveFromISR(ch->sem_sw_int, NULL);
 *		}
 *	}
 *
 *	static void _mtk_os_hal_mbox_ca7_sw_int_irq_handler(void)
 *	{
 *		_mtk_os_hal_mbox_sw_int_irq_handler(OS_HAL_MBOX_CH0);
 *	}
 *
 *	static void _mtk_os_hal_mbox_io_sw_int_irq_handler(void)
 *	{
 *		_mtk_os_hal_mbox_sw_int_irq_handler(OS_HAL_MBOX_CH1);
 *	}
 *
 *	static void _mtk_os_hal_mbox_fifo_irq_handler(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		struct mtk_os_hal_mbox_cb_data cb_data;
 *		void __iomem *base;
 *		struct mbox_fifo_event status;
 *		struct mbox_int_arg int_ctrl;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return;
 *		}
 *		base = ch->base;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOGET_INT_EN, &(ch->mask));
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOGET_INT_STS, &status);
 *
 *		ch->event.channel = channel;
 *		ch->event.rd_int = ch->mask.rd_int && status.rd_int;
 *		ch->event.nf_sts = ch->mask.nf_sts && status.nf_sts;
 *		ch->event.wr_int = ch->mask.wr_int && status.wr_int;
 *		ch->event.ne_sts = ch->mask.ne_sts && status.ne_sts;
 *
 *		if (ch->event.rd_int) {
 *			int_ctrl.type = MBOX_INT_TYPE_RD;
 *			mtk_mhal_mbox_ioctl(base,
 *					MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		}
 *
 *		if (status.nf_sts)
 *			NVIC_DisableIRQ(nf_vectors[channel]);
 *
 *		if (ch->event.wr_int) {
 *			int_ctrl.type = MBOX_INT_TYPE_WR;
 *			mtk_mhal_mbox_ioctl(base,
 *					MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		}
 *
 *		if (status.ne_sts)
 *			NVIC_DisableIRQ(ne_vectors[channel]);
 *
 *		memcpy(&(cb_data.event), &(ch->event),
 *					sizeof(struct mbox_fifo_event));
 *
 *		if (cb_data.event.rd_int || cb_data.event.nf_sts
 *			|| cb_data.event.wr_int || cb_data.event.ne_sts) {
 *			xSemaphoreTakeFromISR(ch->sem_fifo_int, NULL);
 *			if (ch->fifo_cb != NULL)
 *				ch->fifo_cb(&cb_data);
 *			xSemaphoreGiveFromISR(ch->sem_fifo_int, NULL);
 *		}
 *	}
 *
 *	static void _mtk_os_hal_mbox_ca7_fifo_irq_handler(void)
 *	{
 *		_mtk_os_hal_mbox_fifo_irq_handler(OS_HAL_MBOX_CH0);
 *	}
 *
 *	static void _mtk_os_hal_mbox_io_fifo_irq_handler(void)
 *	{
 *		_mtk_os_hal_mbox_fifo_irq_handler(OS_HAL_MBOX_CH1);
 *	}
 *
 *	int mtk_os_hal_mbox_sw_int_register_cb(mbox_channel_t channel,
 *				mtk_os_hal_mbox_cb cb, u32 irq_status)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *		u32 SW_INT_MASK = ((1 << MBOX_NUM_SW_INT) - 1);
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *
 *		if (cb == NULL) {
 *			OS_MBOX_ERROR("cb is null\n");
 *			return -MBOX_EPTR;
 *		}
 *
 *		if (irq_status > SW_INT_MASK) {
 *			OS_MBOX_ERROR("invalid irq_status:0x%x\n", irq_status);
 *			return -MBOX_EDEFAULT;
 *		}
 *
 *		base = ch->base;
 *
 *		xSemaphoreTake(ch->sem_sw_int, portMAX_DELAY);
 *		ch->swint_cb = cb;
 *		ch->swint_en = irq_status;
 *
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_SWINT, &SW_INT_MASK);
 *
 *		NVIC_DisableIRQ(sw_vectors[channel]);
 *		if (channel == OS_HAL_MBOX_CH0)
 *			NVIC_Register(sw_vectors[channel],
 *				_mtk_os_hal_mbox_ca7_sw_int_irq_handler);
 *		else if (channel == OS_HAL_MBOX_CH1)
 *			NVIC_Register(sw_vectors[channel],
 *				_mtk_os_hal_mbox_io_sw_int_irq_handler);
 *		NVIC_SetPriority(sw_vectors[channel], 5);
 *		NVIC_EnableIRQ(sw_vectors[channel]);
 *
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_SWINT_EN, &irq_status);
 *
 *		xSemaphoreGive(ch->sem_sw_int);
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_sw_int_unregister_cb(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *		u32 SW_INT_MASK = ((1 << MBOX_NUM_SW_INT) - 1);
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *		base = ch->base;
 *
 *		xSemaphoreTake(ch->sem_sw_int, portMAX_DELAY);
 *		ch->swint_cb = NULL;
 *		ch->swint_en = 0x0;
 *
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_SWINT_EN, &(ch->swint_en));
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_SWINT, &SW_INT_MASK);
 *		NVIC_DisableIRQ(sw_vectors[channel]);
 *
 *		xSemaphoreGive(ch->sem_sw_int);
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_fifo_register_cb(mbox_channel_t channel,
 *				mtk_os_hal_mbox_cb cb,
 *				struct mbox_fifo_event *mask)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *		struct mbox_int_arg int_ctrl;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *
 *		if (cb == NULL) {
 *			OS_MBOX_ERROR("cb is null\n");
 *			return -MBOX_EPTR;
 *		}
 *
 *		base = ch->base;
 *		xSemaphoreTake(ch->sem_fifo_int, portMAX_DELAY);
 *		ch->fifo_cb = cb;
 *
 *		NVIC_DisableIRQ(wr_vectors[channel]);
 *		NVIC_DisableIRQ(rd_vectors[channel]);
 *		NVIC_DisableIRQ(ne_vectors[channel]);
 *		NVIC_DisableIRQ(nf_vectors[channel]);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_NE;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		int_ctrl.enable = mask->ne_sts;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_WR;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		int_ctrl.enable = mask->wr_int;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_NF;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		int_ctrl.enable = mask->nf_sts;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_RD;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		int_ctrl.enable = mask->rd_int;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		if (channel == OS_HAL_MBOX_CH0) {
 *			CM4_Install_NVIC(wr_vectors[channel], 5,
 *				IRQ_EDGE_TRIGGER,
 *				_mtk_os_hal_mbox_ca7_fifo_irq_handler,
 *				mask->wr_int);
 *			CM4_Install_NVIC(rd_vectors[channel], 5,
 *				IRQ_EDGE_TRIGGER,
 *				_mtk_os_hal_mbox_ca7_fifo_irq_handler,
 *				mask->rd_int);
 *			CM4_Install_NVIC(ne_vectors[channel], 5,
 *				IRQ_LEVEL_TRIGGER,
 *				_mtk_os_hal_mbox_ca7_fifo_irq_handler,
 *				mask->ne_sts);
 *			CM4_Install_NVIC(nf_vectors[channel], 5,
 *				IRQ_LEVEL_TRIGGER,
 *				_mtk_os_hal_mbox_ca7_fifo_irq_handler,
 *				mask->nf_sts);
 *		} else if (channel == OS_HAL_MBOX_CH1) {
 *			CM4_Install_NVIC(wr_vectors[channel], 5,
 *				IRQ_EDGE_TRIGGER,
 *				_mtk_os_hal_mbox_io_fifo_irq_handler,
 *				mask->wr_int);
 *			CM4_Install_NVIC(rd_vectors[channel], 5,
 *				IRQ_EDGE_TRIGGER,
 *				_mtk_os_hal_mbox_io_fifo_irq_handler,
 *				mask->rd_int);
 *			CM4_Install_NVIC(ne_vectors[channel], 5,
 *				IRQ_LEVEL_TRIGGER,
 *				_mtk_os_hal_mbox_io_fifo_irq_handler,
 *				mask->ne_sts);
 *			CM4_Install_NVIC(nf_vectors[channel], 5,
 *				IRQ_LEVEL_TRIGGER,
 *				_mtk_os_hal_mbox_io_fifo_irq_handler,
 *				mask->nf_sts);
 *		}
 *
 *		xSemaphoreGive(ch->sem_fifo_int);
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_fifo_unregister_cb(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *		struct mbox_int_arg int_ctrl;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *		base = ch->base;
 *
 *		xSemaphoreTake(ch->sem_fifo_int, portMAX_DELAY);
 *
 *		ch->fifo_cb = NULL;
 *
 *		int_ctrl.enable = 0;
 *
 *		int_ctrl.type = MBOX_INT_TYPE_NE;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_WR;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_NF;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		int_ctrl.type = MBOX_INT_TYPE_RD;
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT, &int_ctrl);
 *		mtk_mhal_mbox_ioctl(base, MBOX_IOSET_INT_EN, &int_ctrl);
 *
 *		NVIC_DisableIRQ(wr_vectors[channel]);
 *		NVIC_DisableIRQ(rd_vectors[channel]);
 *		NVIC_DisableIRQ(ne_vectors[channel]);
 *		NVIC_DisableIRQ(nf_vectors[channel]);
 *
 *		xSemaphoreGive(ch->sem_fifo_int);
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_open_channel(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *
 *		if (channel >= OS_HAL_MBOX_CH_MAX
 *			|| channel < OS_HAL_MBOX_CH0) {
 *			OS_MBOX_ERROR(
 *				"channel:%d out of range, should be:0~%d\n",
 *				channel, OS_HAL_MBOX_CH_MAX-1);
 *			return -MBOX_EDEFAULT;
 *		}
 *
 *		ch = &mbox_dev->channels[channel];
 *
 *		if (ch->enable) {
 *			OS_MBOX_ERROR(" channel %d already opened\n", channel);
 *			return MBOX_OK;
 *		}
 *
 *		ch->base = (void __iomem *)mbox_base_addr[channel];
 *		base = ch->base;
 *
 *		vSemaphoreCreateBinary(ch->sem_sw_int);
 *		vSemaphoreCreateBinary(ch->sem_fifo_int);
 *		vSemaphoreCreateBinary(ch->sem_fifo_read);
 *		vSemaphoreCreateBinary(ch->sem_fifo_write);
 *
 *		mtk_mhal_mbox_reset(base);
 *		ch->enable = 1;
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_close_channel(mbox_channel_t channel)
 *	{
 *		struct os_hal_mbox_channel *ch;
 *
 *		if (channel >= OS_HAL_MBOX_CH_MAX
 *			|| channel < OS_HAL_MBOX_CH0) {
 *			OS_MBOX_ERROR(
 *				"channel:%d out of range, should be:0~%d\n",
 *				channel, OS_HAL_MBOX_CH_MAX-1);
 *			return -MBOX_EDEFAULT;
 *		}
 *
 *		ch = &mbox_dev->channels[channel];
 *
 *		if (!ch->enable) {
 *			OS_MBOX_ERROR(" channel %d already closed\n", channel);
 *			return MBOX_OK;
 *		}
 *
 *		vSemaphoreDelete(ch->sem_sw_int);
 *		vSemaphoreDelete(ch->sem_fifo_int);
 *		vSemaphoreDelete(ch->sem_fifo_read);
 *		vSemaphoreDelete(ch->sem_fifo_write);
 *
 *		ch->base = NULL;
 *		ch->enable = 0;
 *
 *		return MBOX_OK;
 *	}
 *
 *	int mtk_os_hal_mbox_fifo_read(mbox_channel_t channel,
 *				struct mbox_fifo_item *buf,
 *				mbox_tr_type_t type)
 *	{
 *		int ret;
 *		struct os_hal_mbox_channel *ch;
 *		struct mbox_int_arg int_ctrl;
 *		void __iomem *base;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *		base = ch->base;
 *
 *		if (pdTRUE !=
 *			xSemaphoreTake(ch->sem_fifo_read, portMAX_DELAY)) {
 *			OS_MBOX_ERROR("take sem_fifo_read fail\n");
 *			return -MBOX_EDEFAULT;
 *		}
 *
 *		ret = mtk_mhal_mbox_fifo_read(base, buf, type);
 *
 *		if (ret == MBOX_OK) {
 *			mtk_mhal_mbox_ioctl(base, MBOX_IOGET_INT_EN, &ch->mask);
 *			if (ch->mask.ne_sts) {
 *				int_ctrl.type = MBOX_INT_TYPE_NE;
 *				mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT,
 *					&int_ctrl);
 *				NVIC_EnableIRQ(ne_vectors[channel]);
 *			}
 *		}
 *
 *		xSemaphoreGive(ch->sem_fifo_read);
 *
 *		return ret;
 *	}
 *
 *	int mtk_os_hal_mbox_fifo_write(mbox_channel_t channel,
 *				const struct mbox_fifo_item *buf,
 *				mbox_tr_type_t type)
 *	{
 *		int ret;
 *		struct os_hal_mbox_channel *ch;
 *		struct mbox_int_arg int_ctrl;
 *		void __iomem *base;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *		base = ch->base;
 *
 *		if (pdTRUE !=
 *			xSemaphoreTake(ch->sem_fifo_write, portMAX_DELAY)) {
 *			OS_MBOX_ERROR("take sem_fifo_write fail\n");
 *			return -MBOX_EDEFAULT;
 *		}
 *
 *		ret = mtk_mhal_mbox_fifo_write(base, buf, type);
 *
 *		if (ret == MBOX_OK) {
 *			mtk_mhal_mbox_ioctl(base, MBOX_IOGET_INT_EN, &ch->mask);
 *			if (ch->mask.nf_sts) {
 *				int_ctrl.type = MBOX_INT_TYPE_NF;
 *				mtk_mhal_mbox_ioctl(base, MBOX_IOSET_CLEAR_INT,
 *					&int_ctrl);
 *				NVIC_EnableIRQ(nf_vectors[channel]);
 *			}
 *		}
 *
 *		xSemaphoreGive(ch->sem_fifo_write);
 *
 *		return ret;
 *	}
 *
 *	int mtk_os_hal_mbox_ioctl(mbox_channel_t channel,
 *				mbox_ioctl_t ctrl, void *arg)
 *	{
 *		int ret;
 *		struct os_hal_mbox_channel *ch;
 *		void __iomem *base;
 *
 *		ch = _mtk_os_hal_mbox_get_channel(channel);
 *		if (ch == NULL) {
 *			OS_MBOX_ERROR("get channel failed\n");
 *			return -MBOX_EPTR;
 *		}
 *		base = ch->base;
 *
 *		if (ctrl == MBOX_IOGET_POST_FIFO_CNT) {
 *			if (pdTRUE !=
 *				xSemaphoreTake(ch->sem_fifo_write,
 *					portMAX_DELAY)) {
 *				OS_MBOX_ERROR("take sem_fifo_write fail\n");
 *				return -MBOX_EDEFAULT;
 *			}
 *		} else if (ctrl == MBOX_IOGET_ACPT_FIFO_CNT) {
 *			if (pdTRUE !=
 *				xSemaphoreTake(ch->sem_fifo_read,
 *					portMAX_DELAY)) {
 *				OS_MBOX_ERROR("take sem_fifo_read fail\n");
 *				return -MBOX_EDEFAULT;
 *			}
 *		}
 *
 *		ret = mtk_mhal_mbox_ioctl(base, ctrl, arg);
 *
 *		if (ctrl == MBOX_IOGET_POST_FIFO_CNT)
 *			xSemaphoreGive(ch->sem_fifo_write);
 *		else if (ctrl == MBOX_IOGET_ACPT_FIFO_CNT)
 *			xSemaphoreGive(ch->sem_fifo_read);
 *
 *		return ret;
 *	}
 *
 *    @endcode
 *
 *
 * - \b Device \b driver \b sample \b code \b is \b as \b follows: \n
 *  - Sample code (Below are the user application sample codes on FreeRTOS):
 *    @code
 *	// First call this function to initialize MBOX HW
 *	mtk_os_hal_mbox_open_channel(OS_HAL_MBOX_CH1);
 *
 *	struct mbox_fifo_event mask;
 *	struct mbox_fifo_item buf;
 *
 *	mask.ne_sts = 0;
 *	mask.wr_int = 1;
 *	mask.nf_sts = 0;
 *	mask.rd_int = 1;
 *
 *	mtk_os_hal_mbox_sw_int_register_cb(OS_HAL_MBOX_CH1,
 *					mbox_cli_io_swint_cb, 0xFF);
 *	mtk_os_hal_mbox_fifo_register_cb(OS_HAL_MBOX_CH1,
 *					mbox_cli_io_fifo_cb, &mask);
 *	buf.data = 0x1;
 *	buf.cmd = 0x2;
 *
 *	// Assuming that IO0 write data
 *	mtk_os_hal_mbox_fifo_write(OS_HAL_MBOX_CH1, &buf, MBOX_TR_DATA_CMD);
 *
 *	// Assuming that IO1 read data
 *	mtk_os_hal_mbox_fifo_read(OS_HAL_MBOX_CH1, &buf, MBOX_TR_DATA_CMD);
 *    @endcode
 *
 *
 * @}
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup MBOX
 * @{
 */

/** @defgroup driver_mbox_define Define
  * @{
  * This section introduces the Macro definition
  * which is used as MBOX M-HAL's and OS-HAL's API error return type.
  */

/** It means the function performs successfully */
#define MBOX_OK				0
/** Refer to the error except the following three errors */
#define MBOX_EDEFAULT			1
/** Invalid argument; it means the pointer is NULL */
#define MBOX_EPTR			2
/** FIFO EMPTY */
#define MBOX_EEMPTY			3
/** FIFO FULL */
#define MBOX_EFULL			4

/**
  * @}
  */

/** @defgroup driver_mbox_enum Enum
  * @{
  * This section introduces the enumerations that are used by MBOX OS-HAL/M-HAL
  * and user application.
  */

/** @brief FIFO interrupt type. */
enum mbox_int_type {
	/** FIFO Non-Empty interrupt */
	MBOX_INT_TYPE_NE,
	/** FIFO Write interrupt */
	MBOX_INT_TYPE_WR,
	/** FIFO Non-Full interrupt */
	MBOX_INT_TYPE_NF,
	/** FIFO Read interrupt */
	MBOX_INT_TYPE_RD,
	/** Invalid */
	MBOX_INT_TYPE_MAX,
};

/** @brief Semaphore status, only used between CA7 and CM4 */
enum mbox_sem_status {
	/** SEM UNLOCK */
	MBOX_SEM_UNLOCK,
	/** SEM CM4 LOCK */
	MBOX_SEM_CM4_LOCK,
	/** SEM CA7 LOCK */
	MBOX_SEM_CA7_LOCK,
	/** Invalid State */
	MBOX_SEM_UNDEF,
};

/** @brief User should specify the transfer type when reading/writing FIFO. */
typedef enum mbox_transfer_type {
	/** Only write/read cmd to/from FIFO. */
	MBOX_TR_CMD_ONLY,
	/** Write/read both data and cmd to/from FIFO. */
	MBOX_TR_DATA_CMD,
	/** Meaningless transfer type */
	MBOX_TR_MAX,
} mbox_tr_type_t;

/** @brief Users should specify the ioctl type when setting/getting information
 *  to/from MBOX HW.
 */
typedef enum mbox_ioctl_type {
	/** Set FIFO Non-Empty threshold. */
	MBOX_IOSET_NE_THRS,
	/** Get FIFO Non-Empty threshold. */
	MBOX_IOGET_NE_THRS,
	/** Set FIFO Non-Full threshold. */
	MBOX_IOSET_NF_THRS,
	/** Get FIFO Non-Full threshold. */
	MBOX_IOGET_NF_THRS,
	/** Release sema. */
	MBOX_IOSET_SEM_POST,
	/** Acquire sema. */
	MBOX_IOSET_SEM_PEND,

	/** Trigger software interrupt. */
	MBOX_IOSET_SWINT_TRIG,
	/** Enable software interrupt. */
	MBOX_IOSET_SWINT_EN,
	/** Get 'enable software interrupt' mask. */
	MBOX_IOGET_SWINT_EN,
	/** Control software interrupt. */
	MBOX_IOSET_SWINT_CTRL,
	/** Get software interrupt status. */
	MBOX_IOGET_SWINT_STS,
	/** Clear software interrupt. */
	MBOX_IOSET_CLEAR_SWINT,

	/** Enable FIFO interrupt. */
	MBOX_IOSET_INT_EN,
	/** Get 'enable FIFO interrupt' mask. */
	MBOX_IOGET_INT_EN,
	/** Get FIFO interrupt status. */
	MBOX_IOGET_INT_STS,
	/** Clear FIFO interrupt. */
	MBOX_IOSET_CLEAR_INT,

	/** Get write FIFO count(CMD FIFO). */
	MBOX_IOGET_POST_FIFO_CNT,
	/** Get read FIFO count(CMD FIFO). */
	MBOX_IOGET_ACPT_FIFO_CNT
} mbox_ioctl_t;

/**
  * @}
  */

/** @defgroup driver_mbox_struct Struct
  * @{
  * This section introduces the structures that are used by MBOX OS-HAL/M-HAL
  * and user application.
  */

/** @brief The data and cmd written to/read from MBOX FIFO. */
struct mbox_fifo_item {
	/** The data written to/read from MBOX FIFO. */
	u32 data;
	/** The cmd written to/read from MBOX FIFO. */
	u32 cmd;
};

/** @brief To enable/disable software interrupt specified by id. */
struct mbox_swint_arg {
	/** Software interrupt enable/disable. */
	int enable;
	/** Software interrupt id. */
	u32 id;
};

/** @brief To enable/disable FIFO interrupt specified by type. */
struct mbox_int_arg {
	/** FIFO interrupt type. */
	enum mbox_int_type type;
	/** FIFO interrupt enable/disable. */
	u32 enable;
};

/**
 *@brief FIFO interrupt status information or enable mask.
 *@brief Usage: If used as FIFO interrupt status, it is output parameter. \n
 *		If used as FIFO interrupt enable mask, it can be input
 *		or output.
 */
struct mbox_fifo_event {
	/** MBOX channel. */
	u32	channel;
	/**
	 * Status: Read FIFO interrupt. \n
	 * Mask: 0: disable, 1: enable.
	 */
	u32	rd_int;
	/**
	 * Status: Write FIFO interrupt. \n
	 * Mask: 0: disable, 1: enable.
	 */
	u32	wr_int;
	/**
	 * Status: FIFO Non-Full interrupt. \n
	 * Mask: 0: disable, 1: enable.
	 */
	u32	nf_sts;
	/**
	 * Status: FIFO Non-Empty interrupt. \n
	 * Mask: 0: disable, 1: enable.
	 */
	u32	ne_sts;
};

/** @brief Software interrupt information. */
struct mbox_swint_info {
	/** MBOX channel. */
	u32	channel;
	/** Software interrupt status. */
	u32	swint_sts;
};

/**
  * @}
  */

/** @defgroup driver_mbox_function Function
  * @{
  * This section provides fixed APIs(defined as Common Interface)
  * to fully control the MediaTek MBOX HW.
  */

/**
 *@brief This function is used to reset MBOX channel.
 *@brief Usage: It is used for OS-HAL to reset MBOX channel when opening
 * channel. After reset, the other end of MBOX should also be reset;
 * otherwise, the MBOX function will be abnormal.
 *@param [in] base : MBOX channel base address.
 *
 *@return
 * Return #MBOX_OK if reset successfully.\n
 * Return -#MBOX_EPTR if base is NULL.
 */
int mtk_mhal_mbox_reset(void __iomem *base);

/**
 *@brief This function is used to read MBOX channel FIFO data.
 *@brief Usage: Reading data and/or cmd from FIFO when receiving
 * write interrupt or non-empty interrupt.
 *@param [in] base : MBOX channel base address.
 *@param [out] buf : Buffer to load data and/or cmd.
 *@param [in] type : Transfer type; see @ref mbox_tr_type_t for details.
 *
 *@return
 * Return #MBOX_OK if reading successfully.\n
 * Return -#MBOX_EPTR if base/buf is NULL.\n
 * Return -#MBOX_EEMPTY if FIFO is empty.\n
 * Return -#MBOX_EDEFAULT if type is invalid.
 */
int mtk_mhal_mbox_fifo_read(void __iomem *base, struct mbox_fifo_item *buf,
			mbox_tr_type_t type);

/**
 *@brief This function is used to write data to MBOX channel FIFO.
 *@brief Usage: Writing data and/or cmd to FIFO.
 *@param [in] base : MBOX channel base address.
 *@param [in] buf : The data and/or cmd to be transferred.
 *@param [in] type : Transfer type; see @ref mbox_tr_type_t for details.
 *
 *@return
 * Return #MBOX_OK if writing successfully.\n
 * Return -#MBOX_EPTR if base/buf is NULL.\n
 * Return -#MBOX_EFULL if FIFO is full.\n
 * Return -#MBOX_EDEFAULT if type is invalid.
 */
int mtk_mhal_mbox_fifo_write(void __iomem *base,
			const struct mbox_fifo_item *buf,
			mbox_tr_type_t type);

/**
 *@brief Control various hardware settings of MBOX
 *@brief Usage: Getting/setting MBOX hardware settings. It is not thread-based,
 * and do NOT call it in multi-thread.
 *@param [in] base : MBOX channel base address.
 *@param [in] ctrl :\n
 *	Operation on Non-Full/Non-Empty threshold values: \n
 *	MBOX_IOSET_NE_THRS,\n
 *	MBOX_IOGET_NE_THRS,\n
 *	MBOX_IOSET_NF_THRS,\n
 *	MBOX_IOGET_NF_THRS. \n
 *	\n
 *	Hardware semaphore operations: \n
 *	MBOX_IOSET_SEM_POST,\n
 *	MBOX_IOSET_SEM_PEND. \n
 *	\n
 *	Software interrupt operations: \n
 *	MBOX_IOSET_SWINT_TRIG,\n
 *	MBOX_IOSET_SWINT_EN,\n
 *	MBOX_IOGET_SWINT_EN,\n
 *	MBOX_IOSET_SWINT_CTRL,\n
 *	MBOX_IOGET_SWINT_STS,\n
 *	MBOX_IOSET_CLEAR_SWINT. \n
 *	\n
 *	FIFO interrupt operations: \n
 *	MBOX_IOSET_INT_EN,\n
 *	MBOX_IOGET_INT_EN,\n
 *	MBOX_IOGET_INT_STS,\n
 *	MBOX_IOSET_CLEAR_INT. \n
 *	\n
 *	Get FIFO count values: \n
 *	MBOX_IOGET_POST_FIFO_CNT,\n
 *	MBOX_IOGET_ACPT_FIFO_CNT.
 *
 *@param [in] arg :\n
 *	MBOX_IOSET_NE_THRS: u32*, threshold value; \n
 *	MBOX_IOSET_NF_THRS: u32*, threshold value; \n
 *	\n
 *	MBOX_IOSET_SEM_POST: not used; \n
 *	\n
 *	MBOX_IOSET_SWINT_TRIG: u32*, interrupt number (0 ~ 7); \n
 *	MBOX_IOSET_SWINT_EN: u32*, interrupt mask (0 ~ 0xFF); \n
 *	MBOX_IOSET_SWINT_CTRL: struct mbox_swint_arg*; \n
 *	MBOX_IOSET_CLEAR_SWINT: u32*, software interrupt mask to be cleared; \n
 *	\n
 *	MBOX_IOSET_INT_EN: struct mbox_int_arg*; \n
 *	MBOX_IOSET_CLEAR_INT: struct mbox_int_arg*; \n
 *	\n
 *
 *@param [out] arg :\n
 *	MBOX_IOGET_NE_THRS: u32*, buffer to load threshold value; \n
 *	MBOX_IOGET_NF_THRS: u32*, buffer to load threshold value; \n
 *	\n
 *	MBOX_IOSET_SEM_PEND: u32*, buffer to load sema status; \n
 *	\n
 *	MBOX_IOGET_SWINT_EN: u32*, buffer to load sw int mask; \n
 *	MBOX_IOGET_SWINT_STS: u32*, buffer to load the status value; \n
 *	\n
 *	MBOX_IOGET_INT_EN: struct mbox_fifo_event*, buffer to load int mask; \n
 *	MBOX_IOGET_INT_STS: struct mbox_fifo_event*, to load int status; \n
 *	\n
 *	MBOX_IOGET_POST_FIFO_CNT: u32*, buffer to load the FIFO count
 *	which indicates how many elements have already been written;\n
 *	MBOX_IOGET_ACPT_FIFO_CNT: u32*, buffer to load the FIFO count
 *	which indicates how many elements can be read. \n
 *
 *@return
 * Return #MBOX_OK if the function operates successfully.\n
 * Return -#MBOX_EPTR if base/arg is NULL.\n
 * Return -#MBOX_EDEFAULT if the function does not operate successfully.
 */
int mtk_mhal_mbox_ioctl(void __iomem *base,
			mbox_ioctl_t ctrl, void *arg);

/**
  * @}
  */

/**
 * @}
 * @}
 */

#endif /* __MHAL_MBOX_H__ */
