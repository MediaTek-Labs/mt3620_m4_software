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

#include "nvic.h"
#include "os_hal_i2c.h"
#include "os_hal_dma.h"
#include "os_hal_gpio.h"

#define MTK_I2C_MAX_PORT_NUMBER 5
#define MTK_I2C_MODE	2

#define ISU0_I2C_BASE	0x38070200
#define ISU1_I2C_BASE	0x38080200
#define ISU2_I2C_BASE	0x38090200
#define ISU3_I2C_BASE	0x380a0200
#define ISU4_I2C_BASE	0x380b0200

#define ISU0_CG_BASE	0x38070000
#define ISU1_CG_BASE	0x38080000
#define ISU2_CG_BASE	0x38090000
#define ISU3_CG_BASE	0x380a0000
#define ISU4_CG_BASE	0x380b0000

static unsigned long i2c_base_addr[MTK_I2C_MAX_PORT_NUMBER] = {
	ISU0_I2C_BASE,
	ISU1_I2C_BASE,
	ISU2_I2C_BASE,
	ISU3_I2C_BASE,
	ISU4_I2C_BASE,
};

static unsigned long cg_base_addr[MTK_I2C_MAX_PORT_NUMBER] = {
	ISU0_CG_BASE,
	ISU1_CG_BASE,
	ISU2_CG_BASE,
	ISU3_CG_BASE,
	ISU4_CG_BASE,
};

static int i2c_dma_chan[MTK_I2C_MAX_PORT_NUMBER][2] = {
	/* [0]:tx, [1]:rx */
	{DMA_ISU0_TX_CH0, DMA_ISU0_RX_CH1},
	{DMA_ISU1_TX_CH2, DMA_ISU1_RX_CH3},
	{DMA_ISU2_TX_CH4, DMA_ISU2_RX_CH5},
	{DMA_ISU3_TX_CH6, DMA_ISU3_RX_CH7},
	{DMA_ISU4_TX_CH8, DMA_ISU4_RX_CH9},
};

/**
 * this os special i2c structure, need mapping it to mtk_i2c_controller
 */
struct mtk_i2c_ctrl_rtos {
	struct mtk_i2c_controller *i2c;

	/* the type based on OS */
	volatile u8 xfer_completion;
};

static struct mtk_i2c_ctrl_rtos g_i2c_ctrl_rtos[MTK_I2C_MAX_PORT_NUMBER];
struct mtk_i2c_controller g_i2c_ctrl[MTK_I2C_MAX_PORT_NUMBER];
struct mtk_i2c_private g_i2c_mdata[MTK_I2C_MAX_PORT_NUMBER];

static int _mtk_os_hal_i2c_pinmux(u32 pin_scl, u32 pin_sda)
{
	int ret = 0;

	ret = mtk_os_hal_gpio_request(pin_scl);
	ret |= mtk_os_hal_gpio_request(pin_sda);

	if (ret < 0) {
		printf("I2C request gpio pin fail, ret = %d\n", ret);
		return ret;
	}

	mtk_os_hal_gpio_pmx_set_mode(pin_scl, MTK_I2C_MODE);
	mtk_os_hal_gpio_pmx_set_mode(pin_sda, MTK_I2C_MODE);

	return 0;
}

static int _mtk_os_hal_i2c_config_pinmux(u8 bus_num)
{
	int ret = 0;

	switch (bus_num) {
	case 0:
		ret = _mtk_os_hal_i2c_pinmux(MHAL_GPIO_27, MHAL_GPIO_28);
		break;
	case 1:
		ret = _mtk_os_hal_i2c_pinmux(MHAL_GPIO_32, MHAL_GPIO_33);
		break;
	case 2:
		ret = _mtk_os_hal_i2c_pinmux(MHAL_GPIO_37, MHAL_GPIO_38);
		break;
	case 3:
		ret = _mtk_os_hal_i2c_pinmux(MHAL_GPIO_67, MHAL_GPIO_68);
		break;
	case 4:
		ret = _mtk_os_hal_i2c_pinmux(MHAL_GPIO_72, MHAL_GPIO_73);
		break;
	}

	return ret;
}

static void _mtk_os_hal_i2c_free_pinmux(u8 bus_num)
{
	switch (bus_num) {
	case 0:
		mtk_os_hal_gpio_free(MHAL_GPIO_27);
		mtk_os_hal_gpio_free(MHAL_GPIO_28);
		break;
	case 1:
		mtk_os_hal_gpio_free(MHAL_GPIO_32);
		mtk_os_hal_gpio_free(MHAL_GPIO_33);
		break;
	case 2:
		mtk_os_hal_gpio_free(MHAL_GPIO_37);
		mtk_os_hal_gpio_free(MHAL_GPIO_38);
		break;
	case 3:
		mtk_os_hal_gpio_free(MHAL_GPIO_67);
		mtk_os_hal_gpio_free(MHAL_GPIO_68);
		break;
	case 4:
		mtk_os_hal_gpio_free(MHAL_GPIO_72);
		mtk_os_hal_gpio_free(MHAL_GPIO_73);
		break;
	}
}

static void _mtk_os_hal_i2c_irq_handler(int bus_num)
{
	u8 ret = 0;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];
	struct mtk_i2c_controller *i2c = ctrl_rtos->i2c;

	ret = mtk_mhal_i2c_irq_handle(i2c);

	/* 1. FIFO mode: return completion done in I2C irq handler
	 * 2. DMA mode: return completion done in DMA irq handler
	 */
	if (!ret) {
		ctrl_rtos->xfer_completion++;
	}
}

static void _mtk_os_hal_i2c0_irq_event(void)
{
	_mtk_os_hal_i2c_irq_handler(0);
}

static void _mtk_os_hal_i2c1_irq_event(void)
{
	_mtk_os_hal_i2c_irq_handler(1);
}

static void _mtk_os_hal_i2c2_irq_event(void)
{
	_mtk_os_hal_i2c_irq_handler(2);
}

static void _mtk_os_hal_i2c3_irq_event(void)
{
	_mtk_os_hal_i2c_irq_handler(3);
}

static void _mtk_os_hal_i2c4_irq_event(void)
{
	_mtk_os_hal_i2c_irq_handler(4);
}

static void _mtk_os_hal_i2c_request_irq(int bus_num)
{
	switch (bus_num) {
	case 0:
		CM4_Install_NVIC(CM4_IRQ_ISU_G0_I2C, DEFAULT_PRI,
				 IRQ_LEVEL_TRIGGER, _mtk_os_hal_i2c0_irq_event,
				 TRUE);
		break;
	case 1:
		CM4_Install_NVIC(CM4_IRQ_ISU_G1_I2C, DEFAULT_PRI,
				 IRQ_LEVEL_TRIGGER, _mtk_os_hal_i2c1_irq_event,
				 TRUE);
		break;
	case 2:
		CM4_Install_NVIC(CM4_IRQ_ISU_G2_I2C, DEFAULT_PRI,
				 IRQ_LEVEL_TRIGGER, _mtk_os_hal_i2c2_irq_event,
				 TRUE);
		break;
	case 3:
		CM4_Install_NVIC(CM4_IRQ_ISU_G3_I2C, DEFAULT_PRI,
				 IRQ_LEVEL_TRIGGER, _mtk_os_hal_i2c3_irq_event,
				 TRUE);
		break;
	case 4:
		CM4_Install_NVIC(CM4_IRQ_ISU_G4_I2C, DEFAULT_PRI,
				 IRQ_LEVEL_TRIGGER, _mtk_os_hal_i2c4_irq_event,
				 TRUE);
		break;
	}
}

static void _mtk_os_hal_i2c_free_irq(int bus_num)
{
	switch (bus_num) {
	case 0:
		NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_ISU_G0_I2C);
		break;
	case 1:
		NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_ISU_G1_I2C);
		break;
	case 2:
		NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_ISU_G2_I2C);
		break;
	case 3:
		NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_ISU_G3_I2C);
		break;
	case 4:
		NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_ISU_G4_I2C);
		break;
	}
}

static int _mtk_os_hal_i2c_dma_done_callback(void *data)
{
	struct mtk_i2c_ctrl_rtos *ctrl_rtos = data;

	ctrl_rtos->xfer_completion++;
	return 0;
}

static int _mtk_os_hal_i2c_wait_for_completion_timeout(
	struct mtk_i2c_ctrl_rtos *ctrl_rtos, int time_ms)
{
	while(ctrl_rtos->xfer_completion==0){}
	ctrl_rtos->xfer_completion--;
	return 0;
}

int _mtk_os_hal_i2c_transfer(struct mtk_i2c_ctrl_rtos *ctrl_rtos, int bus_num)
{
	int ret = I2C_OK;
	struct mtk_i2c_controller *i2c;

	if (ctrl_rtos == NULL) {
		printf("i2c%d ctrl_rtos is NULL point\n", bus_num);
		return -I2C_EPTR;
	}

	i2c = ctrl_rtos->i2c;
	if (i2c == NULL) {
		printf("i2c%d *i2c is NULL point\n", bus_num);
		return -I2C_EPTR;
	}

	ret = mtk_mhal_i2c_trigger_transfer(i2c);
	if (ret) {
		printf("i2c%d trigger transfer fail\n", bus_num);
		goto err_exit;
	}

	ret = _mtk_os_hal_i2c_wait_for_completion_timeout(ctrl_rtos,
						  i2c->timeout);
	if (ret) {
		printf("Take i2c%d Semaphore timeout!\n", bus_num);
		ret = -I2C_ETIMEDOUT;
	} else
		ret = mtk_mhal_i2c_result_handle(i2c);

	if (ret) {
		mtk_mhal_i2c_dump_register(i2c);
		mtk_mhal_i2c_init_hw(i2c);
	}

err_exit:

	return ret;
}

int mtk_os_hal_i2c_ctrl_init(int bus_num)
{
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;
	int ret = 0;

	if (bus_num >= MTK_I2C_MAX_PORT_NUMBER)
		return -I2C_EINVAL;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = &g_i2c_ctrl[bus_num];
	i2c->base = (void __iomem *)i2c_base_addr[bus_num];
	i2c->cg_base = (void __iomem *)cg_base_addr[bus_num];
	i2c->mdata = &g_i2c_mdata[bus_num];
	i2c->dma_tx_chan = i2c_dma_chan[bus_num][0];
	i2c->dma_rx_chan = i2c_dma_chan[bus_num][1];

	mtk_mhal_i2c_dma_done_callback_register(i2c,
					      _mtk_os_hal_i2c_dma_done_callback,
					     (void *)ctrl_rtos);

	ctrl_rtos->i2c = i2c;
	ctrl_rtos->xfer_completion = 0;

	ret = _mtk_os_hal_i2c_config_pinmux(bus_num);
	if (ret < 0) {
		printf("I2C%d config i2c pinmux fail, ret = %d\n",
			bus_num, ret);
		return ret;
	}

	ret = mtk_mhal_i2c_request_dma(i2c);
	if (ret < 0) {
		printf("I2C%d request dma channel fail, ret = %d\n",
				bus_num, ret);
		_mtk_os_hal_i2c_free_pinmux(bus_num);
		return ret;
	}

	_mtk_os_hal_i2c_request_irq(bus_num);
	mtk_mhal_i2c_enable_clk(i2c);

	return 0;
}

int mtk_os_hal_i2c_ctrl_deinit(int bus_num)
{
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];
	i2c = ctrl_rtos->i2c;

	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	_mtk_os_hal_i2c_free_irq(bus_num);
	mtk_mhal_i2c_release_dma(i2c);
	_mtk_os_hal_i2c_free_pinmux(bus_num);
	mtk_mhal_i2c_disable_clk(i2c);

	i2c = NULL;
	ctrl_rtos->i2c = i2c;

	return 0;
}

int mtk_os_hal_i2c_speed_init(u8 bus_num, enum i2c_speed_kHz speed)
{
	int ret = I2C_OK;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	ret = mtk_mhal_i2c_init_speed(i2c, speed);
	if (ret)
		printf("i2c%d init speed fail\n", bus_num);

	return ret;
}

int mtk_os_hal_i2c_read(u8 bus_num, u8 device_addr, u8 *buffer, u16 len)
{
	struct i2c_msg msgs;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;
	int ret = I2C_OK;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->msg_num = 1;
	i2c->dma_en = false;
	i2c->i2c_mode = I2C_MASTER_MODE;
	i2c->timeout = 2000;
	i2c->irq_stat = 0;

	msgs.addr = device_addr;
	msgs.flags = I2C_MASTER_RD;
	msgs.len = len;
	msgs.buf = buffer;

	i2c->msg = &msgs;

	ret = _mtk_os_hal_i2c_transfer(ctrl_rtos, bus_num);
	if (ret)
		printf("i2c%d read fail\n", bus_num);

	return ret;
}

int   mtk_os_hal_i2c_write(u8 bus_num, u8 device_addr, u8 *buffer, u16 len)
{
	struct i2c_msg msgs;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;
	int ret = I2C_OK;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->msg_num = 1;
	i2c->dma_en = false;
	i2c->i2c_mode = I2C_MASTER_MODE;
	i2c->timeout = 2000;
	i2c->irq_stat = 0;

	msgs.addr = device_addr;
	msgs.flags = I2C_MASTER_WR;
	msgs.len = len;
	msgs.buf = buffer;

	i2c->msg = &msgs;

	ret = _mtk_os_hal_i2c_transfer(ctrl_rtos, bus_num);
	if (ret)
		printf("i2c%d write fail\n", bus_num);

	return ret;
}

int mtk_os_hal_i2c_write_read(u8 bus_num, u8 device_addr,
			      u8 *wr_buf, u8 *rd_buf, u16 wr_len, u16 rd_len)
{
	struct i2c_msg msgs[2];
	struct mtk_i2c_ctrl_rtos *ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];
	struct mtk_i2c_controller *i2c;
	int ret = I2C_OK;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->msg_num = 2;
	i2c->dma_en = false;
	i2c->i2c_mode = I2C_MASTER_MODE;
	i2c->timeout = 2000;
	i2c->irq_stat = 0;

	msgs[0].addr = device_addr;
	msgs[0].flags = I2C_MASTER_WR;
	msgs[0].len = wr_len;
	msgs[0].buf = wr_buf;

	msgs[1].addr = device_addr;
	msgs[1].flags = I2C_MASTER_RD;
	msgs[1].len = rd_len;
	msgs[1].buf = rd_buf;

	i2c->msg = &msgs[0];

	ret = _mtk_os_hal_i2c_transfer(ctrl_rtos, bus_num);
	if (ret)
		printf("i2c%d write fail\n", bus_num);

	return ret;
}

int mtk_os_hal_i2c_set_slave_addr(u8 bus_num, u8 slv_addr)
{
	int ret = I2C_OK;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->i2c_mode = I2C_SLAVE_MODE;

	ret = mtk_mhal_i2c_init_slv_addr(i2c, slv_addr);
	if (ret)
		printf("i2c%d init slv_addr fail\n", bus_num);

	return ret;
}

int mtk_os_hal_i2c_slave_tx(u8 bus_num, u8 *buffer, u16 len, u32 time_out)
{
	struct i2c_msg msgs;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;
	int ret = I2C_OK;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->msg_num = 1;
	i2c->dma_en = false;
	i2c->i2c_mode = I2C_SLAVE_MODE;
	i2c->irq_stat = 0;
	i2c->timeout = time_out;

	msgs.flags = I2C_SLAVE_TX;
	msgs.len = len;
	msgs.buf = buffer;

	i2c->msg = &msgs;

	ret = _mtk_os_hal_i2c_transfer(ctrl_rtos, bus_num);
	if (ret)
		printf("i2c%d slave TX fail\n", bus_num);

	return ret;
}

int mtk_os_hal_i2c_slave_rx(u8 bus_num, u8 *buffer, u16 len, u32 time_out)
{

	struct i2c_msg msgs;
	struct mtk_i2c_ctrl_rtos *ctrl_rtos;
	struct mtk_i2c_controller *i2c;
	int ret = I2C_OK;

	ctrl_rtos = &g_i2c_ctrl_rtos[bus_num];

	i2c = ctrl_rtos->i2c;
	if (!i2c) {
		printf("i2c%d *i2c is NULL Pointer\n", bus_num);
		return -I2C_EPTR;
	}

	i2c->msg_num = 1;
	i2c->dma_en = false;
	i2c->i2c_mode = I2C_SLAVE_MODE;
	i2c->irq_stat = 0;
	i2c->timeout = time_out;
	i2c->msg = &msgs;

	memset(&msgs, 0, sizeof(struct i2c_msg));

	msgs.flags = I2C_SLAVE_RX;
	msgs.buf = buffer;
	msgs.len = len;

	ret = _mtk_os_hal_i2c_transfer(ctrl_rtos, bus_num);
	if (ret) {
		printf("i2c%d slave RX fail\n", bus_num);
		return ret;
	}

	return 0;
}

#define I2C_SLAVE_TX 0xF1
#define I2C_SLAVE_RX 0XF0
#define I2C_SLV_CMD_LEN 2

int mtk_os_hal_i2c_slave_tx_rx(u8 bus_num, u8 *wr_buf, u8 *rd_buf,
		u16 wr_buf_size, u16 *rd_len, u32 time_out)
{
	u8 cmd_buf[2] = {0};
	int ret = 0;

	ret = mtk_os_hal_i2c_slave_rx(bus_num, cmd_buf,
				      I2C_SLV_CMD_LEN, time_out);

	if (ret < 0) {
		printf("i2c slave receive command fail\n");
		return ret;
	}

	switch (cmd_buf[0]) {
	case I2C_SLAVE_TX:
		if (!wr_buf) {
			printf("I2C slave TX buffer NULL!\n");
			return -I2C_EPTR;
		}

		if (wr_buf_size < cmd_buf[1]) {
			printf("i2c slave buffer length(%d) less than master will Read length(%d)!\n",
				wr_buf_size, cmd_buf[1]);
			return -I2C_EINVAL;
		}

		ret = mtk_os_hal_i2c_slave_tx(bus_num, wr_buf,
					      cmd_buf[1], time_out);
		if (ret < 0)
			printf("i2c slave Tx data to master fail!\n");

		break;

	case I2C_SLAVE_RX:
		if (rd_buf == NULL)
			return -I2C_EPTR;
		else
			memset(rd_buf, 0, cmd_buf[1]);

		*rd_len = cmd_buf[1];

		ret = mtk_os_hal_i2c_slave_rx(bus_num, rd_buf,
					      cmd_buf[1], time_out);
		if (ret < 0)
			printf("i2c slave receive data fail!\n");

		break;

	default:
		printf("i2c slave receive command not support!\n");
	}

	return ret;

}
