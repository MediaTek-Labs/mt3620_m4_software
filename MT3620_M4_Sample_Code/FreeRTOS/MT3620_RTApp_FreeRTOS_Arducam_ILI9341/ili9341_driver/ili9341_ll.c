#if defined(AzureSphere_CA7)

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "applibs_versions.h"

#include <applibs/log.h>
#include <applibs/i2c.h>
#include <applibs/spi.h>
#include <applibs/gpio.h>
#include <hw/sample_hardware.h>

static int RstGpioFd;
static int DcGpioFd;
static int BlGpioFd;
static int i2cFd;
static int spiFd;

#define MAX_SPI_TRANSFER_BYTES	4096

#elif defined(AzureSphere_CM4)
#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"

#include "os_hal_gpio.h"
#include "os_hal_spim.h"

#define SPI_MAX_BUF_SIZE 32

extern spim_num spi_master_port_num_ili9341;
extern uint32_t spi_master_speed_khz_ili9341;	/* 40MHz */
extern struct mtk_spi_config spi_default_config_ili9341;
extern const os_hal_gpio_pin gpio_ili9341_rst;
extern const os_hal_gpio_pin gpio_ili9341_dc;
extern const os_hal_gpio_pin gpio_ili9341_bl;
static struct mtk_spi_transfer spim_xfer;
static uint8_t *spim_tx_buf;
static uint8_t *spim_rx_buf;
#endif

#include "ili9341_ll.h"

void ili9341_ll_reset_low(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(RstGpioFd, GPIO_Value_Low);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_rst, OS_HAL_GPIO_DATA_LOW);

#endif
}
void ili9341_ll_reset_high(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(RstGpioFd, GPIO_Value_High);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_rst, OS_HAL_GPIO_DATA_HIGH);

#endif
}

void ili9341_ll_dc_low(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(DcGpioFd, GPIO_Value_Low);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_dc, OS_HAL_GPIO_DATA_LOW);

#endif
}

void ili9341_ll_dc_high(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(DcGpioFd, GPIO_Value_High);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_dc, OS_HAL_GPIO_DATA_HIGH);

#endif
}

void ili9341_ll_backlight_off(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(BlGpioFd, GPIO_Value_Low);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_bl, OS_HAL_GPIO_DATA_LOW);

#endif
}

void ili9341_ll_backlight_on(void)
{
#if defined(AzureSphere_CA7)

	GPIO_SetValue(BlGpioFd, GPIO_Value_High);

#elif defined(AzureSphere_CM4)

	mtk_os_hal_gpio_set_output(gpio_ili9341_bl, OS_HAL_GPIO_DATA_HIGH);

#endif
}

int ili9341_ll_init(void)
{
#if defined(AzureSphere_CA7)

	RstGpioFd = GPIO_OpenAsOutput(ILI9341_RST, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (RstGpioFd < 0) {
		Log_Debug("ERROR: GPIO_OpenAsOutput: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	DcGpioFd = GPIO_OpenAsOutput(ILI9341_DC, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (DcGpioFd < 0) {
		Log_Debug("ERROR: GPIO_OpenAsOutput: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	BlGpioFd = GPIO_OpenAsOutput(ILI9341_BL, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	if (BlGpioFd < 0) {
		Log_Debug("ERROR: GPIO_OpenAsOutput: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	SPIMaster_Config config;
	int ret = SPIMaster_InitConfig(&config);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_InitConfig: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	config.csPolarity = SPI_ChipSelectPolarity_ActiveLow;
	spiFd = SPIMaster_Open(ILI9341_SPI, MT3620_SPI_CS_A, &config);
	if (spiFd < 0) {
		Log_Debug("ERROR: SPIMaster_Open: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	int result = SPIMaster_SetBusSpeed(spiFd, 40000000);
	if (result < 0) {
		Log_Debug("ERROR: SPIMaster_SetBusSpeed: errno=%d (%s)\r\n", errno, strerror(errno));
		close(spiFd);
		return -1;
	}

	result = SPIMaster_SetMode(spiFd, SPI_Mode_0);
	if (result < 0) {
		Log_Debug("ERROR: SPIMaster_SetMode: errno=%d (%s)\r\n", errno, strerror(errno));
		close(spiFd);
		return -1;
	}

	return 0;

#elif defined(AzureSphere_CM4)
	mtk_os_hal_gpio_request(gpio_ili9341_rst);
	mtk_os_hal_gpio_request(gpio_ili9341_dc);
	mtk_os_hal_gpio_request(gpio_ili9341_bl);
	mtk_os_hal_gpio_set_direction(gpio_ili9341_rst, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_direction(gpio_ili9341_dc, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_direction(gpio_ili9341_bl, OS_HAL_GPIO_DIR_OUTPUT);
	ili9341_ll_backlight_on();

	mtk_os_hal_spim_ctlr_init(spi_master_port_num_ili9341);

	spim_tx_buf = pvPortMalloc(SPI_MAX_BUF_SIZE);
	spim_rx_buf = pvPortMalloc(SPI_MAX_BUF_SIZE);

	if (!spim_tx_buf || !spim_rx_buf) {
		while (1) {
			printf("SPI TX/RX Malloc fail! Need check linker.ld configuration.\n");
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	memset(&spim_xfer, 0, sizeof(spim_xfer));
	memset(spim_tx_buf, 0, SPI_MAX_BUF_SIZE);
	memset(spim_rx_buf, 0, SPI_MAX_BUF_SIZE);
	spim_xfer.tx_buf = spim_tx_buf;
	spim_xfer.rx_buf = spim_rx_buf;
	spim_xfer.use_dma = 0;
	spim_xfer.speed_khz = spi_master_speed_khz_ili9341;

	return 0;
#endif
}

int ili9341_ll_spi_tx_u8(uint8_t data)
{
#if defined(AzureSphere_CA7)

	SPIMaster_Transfer transfers;

	int ret = SPIMaster_InitTransfers(&transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_InitTransfers: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	transfers.flags = SPI_TransferFlags_Write;
	transfers.writeData = &data;
	transfers.length = 1;

	ret = SPIMaster_TransferSequential(spiFd, &transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	return 0;

#elif defined(AzureSphere_CM4)
	int ret = 0;

	spim_xfer.tx_buf = spim_tx_buf;
	spim_xfer.rx_buf = NULL;
	spim_xfer.opcode = data;
	spim_xfer.opcode_len = 1;
	spim_xfer.len = 0;

	ret = mtk_os_hal_spim_transfer(spi_master_port_num_ili9341, &spi_default_config_ili9341, &spim_xfer);
	if (ret) {
		printf("mtk_os_hal_spim_transfer failed\n");
		return ret;
	}

	return ret;
#endif
}

int ili9341_ll_spi_rx_u8(uint8_t *data)
{
#if defined(AzureSphere_CA7)

	SPIMaster_Transfer transfers;

	int ret = SPIMaster_InitTransfers(&transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_InitTransfers: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	transfers.flags = SPI_TransferFlags_Read;
	transfers.readData = data;
	transfers.length = 1;

	ret = SPIMaster_TransferSequential(spiFd, &transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	return 0;

#elif defined(AzureSphere_CM4)
	int ret = 0;

	memset(spim_rx_buf, 0, SPI_MAX_BUF_SIZE);
	spim_xfer.tx_buf = NULL;
	spim_xfer.rx_buf = spim_rx_buf;
	spim_xfer.opcode_len = 0;
	spim_xfer.len = 1;

	ret = mtk_os_hal_spim_transfer(spi_master_port_num_ili9341, &spi_default_config_ili9341, &spim_xfer);
	if (ret)
		printf("mtk_os_hal_spim_transfer failed\n");
	else
		*data = spim_rx_buf[0];

	return ret;
#endif
}

int ili9341_ll_spi_tx_u16(uint16_t data)
{
#if defined(AzureSphere_CA7)
	uint8_t buf[2];
	buf[0] = data >> 8;
	buf[1] = data;

	SPIMaster_Transfer transfers;

	int ret = SPIMaster_InitTransfers(&transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_InitTransfers: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	transfers.flags = SPI_TransferFlags_Write;
	transfers.writeData = &buf[0];
	transfers.length = 2;

	ret = SPIMaster_TransferSequential(spiFd, &transfers, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	return 0;

#elif defined(AzureSphere_CM4)
	int ret = 0;

	memset(spim_tx_buf, 0, SPI_MAX_BUF_SIZE);
	spim_xfer.tx_buf = spim_tx_buf;
	spim_xfer.rx_buf = NULL;
	spim_xfer.opcode = ((data >> 8) & 0xff);
	spim_xfer.opcode_len = 1;
	spim_xfer.len = 1;
	spim_tx_buf[0] = data & 0xff;

	ret = mtk_os_hal_spim_transfer(spi_master_port_num_ili9341, &spi_default_config_ili9341, &spim_xfer);
	if (ret) {
		printf("mtk_os_hal_spim_transfer failed\n");
		return ret;
	}
	return ret;
#endif
}

int ili9341_ll_spi_tx(uint8_t *p_data, uint32_t tx_len)
{
#if defined(AzureSphere_CA7)

	uint32_t numOfXfer = tx_len / MAX_SPI_TRANSFER_BYTES;
	uint32_t lastXferSize = tx_len % MAX_SPI_TRANSFER_BYTES;

	SPIMaster_Transfer transfer;
	int ret = SPIMaster_InitTransfers(&transfer, 1);
	if (ret < 0) {
		Log_Debug("ERROR: SPIMaster_InitTransfers: errno=%d (%s)\r\n", errno, strerror(errno));
		return -1;
	}

	transfer.flags = SPI_TransferFlags_Write;
	transfer.length = MAX_SPI_TRANSFER_BYTES;

	for (uint32_t i = 0; i < numOfXfer; i++) {

		transfer.writeData = p_data;

		ret = SPIMaster_TransferSequential(spiFd, &transfer, 1);
		if (ret < 0) {
			Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\r\n", errno, strerror(errno));
			return -1;
		} else if (ret != transfer.length) {
			Log_Debug("ERROR: SPIMaster_TransferSequential transfer %d bytes, expect %d bytes\r\n", ret, transfer.length);
			return -1;
		}

		p_data += MAX_SPI_TRANSFER_BYTES;
	}

	if (lastXferSize > 0) {
		transfer.writeData = p_data;
		transfer.length = lastXferSize;

		ret = SPIMaster_TransferSequential(spiFd, &transfer, 1);
		if (ret < 0) {
			Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\r\n", errno, strerror(errno));
			return -1;
		}
		else if (ret != transfer.length) {
			Log_Debug("ERROR: SPIMaster_TransferSequential transfer %d bytes, expect %d bytes\r\n", ret, transfer.length);
			return -1;
		}
	}

	return 0;

#elif defined(AzureSphere_CM4)
	int ret = 0;
	uint32_t transfer_len = 0;
	uint32_t index = 0;

	while (tx_len > 0) {
		transfer_len = tx_len > SPI_MAX_BUF_SIZE ? SPI_MAX_BUF_SIZE : tx_len;
		
		memset(spim_tx_buf, 0, SPI_MAX_BUF_SIZE);
		memcpy(spim_tx_buf, &p_data[index], transfer_len);
		spim_xfer.tx_buf = &spim_tx_buf[1];
		spim_xfer.rx_buf = NULL;
		spim_xfer.opcode = spim_tx_buf[0];
		spim_xfer.opcode_len = 1;
		spim_xfer.len = transfer_len-1;

		ret = mtk_os_hal_spim_transfer(spi_master_port_num_ili9341, &spi_default_config_ili9341, &spim_xfer);
		if (ret) {
			printf("mtk_os_hal_spim_transfer failed\n");
			return ret;
		}

		tx_len -= transfer_len;
		index += transfer_len;
	}

	return ret;
#endif
}

