// cor-app
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_error.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"
#include "nrf_twi_mngr.h"

#include "buckler.h"
#include "display.h"

#include "kobukiActuator.h"
#include "kobukiSensorTypes.h"
#include "kobukiSensorPoll.h"
#include "kobukiUtilities.h"

#include "lsm9ds1.h"
#include "cor-app.h"

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// UART Serial configuration
NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
							   BUCKLER_UART_RX, BUCKLER_UART_TX,
							   0, 0,
							   NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
							   NRF_UART_BAUDRATE_115200,
							   UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 4096
#define SERIAL_FIFO_RX_SIZE 4096

NRF_SERIAL_QUEUES_DEF(serial_queue, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 128
#define SERIAL_BUFF_RX_SIZE 128

NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_DMA,
					  &serial_queue, &serial_buffs, ser_event_handler, NULL);

NRF_SERIAL_UART_DEF(serial_uart, 0);

// UART over Virtual COM port
uint8_t uart_rx_state = 0; // 0 - nothing read
						   // 1 - 'g''s'
						   // 2 - num_bytes found
						   // 3 - done reading
uint16_t uart_rx_write_head = 0;
uint8_t solution_buffer[4096];
uint16_t solution_num_bytes;

void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event)
{
	switch (event)
	{
	case NRF_SERIAL_EVENT_TX_DONE:
	{
		break;
	}
	case NRF_SERIAL_EVENT_RX_DATA:
	{
		size_t read;
		uint8_t buffer[16];
		nrf_serial_read(&serial_uart, &buffer, sizeof(buffer), &read, 0);
		ser_rx_data(buffer, read);
		break;
	}
	case NRF_SERIAL_EVENT_DRV_ERR:
	{
		nrf_serial_rx_drain(&serial_uart);
		nrf_serial_uninit(&serial_uart);
		nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
		break;
	}
	case NRF_SERIAL_EVENT_FIFO_ERR:
	{
		break;
	}
	}
}

void ser_rx_data(uint8_t *data, size_t size)
{
	// Determine if path-stream solution being transmitted
	// packet header: 'g' 's'
	// packet footer: '\n'

	printf("%.*s\n", size, data); // print the received packets

	uint8_t fc_found = 0; // first char found flag
	uint8_t lc_found = 0; // last char found flag
	// TODO: if fc/lc are not within contiguous 16 bytes, we will not detect beginning or end
	// consider making these two vars global

	for (uint32_t i = 0; i < size; i++)
	{
		switch (uart_rx_state)
		{
		case 0: // Look for 'g' -> 's'
		{

			if (data[i] == 'g')
				fc_found = 1; // found first g
			else if (fc_found == 1 && data[i] == 's')
				uart_rx_state = 1; // found 'g' then 's'
			else if (fc_found == 1 && data[i] != 's')
				fc_found = 0;
			break;
		}
		case 1:
		{
			// Read num_bytes
			// little endian
			solution_num_bytes = data[i] | (data[i + 1] << 8); // unsafe read
			printf("Total num_bytes in solution : %hu\n", solution_num_bytes);
			uart_rx_state = 2;
			i += 1; // skip next byte
			break;
		}
		case 2:
		{
			if (uart_rx_write_head + 4 == solution_num_bytes)
			{	// write_head starts after first 4 bytes passed
				// TODO: check for \n here?
				uart_rx_state = 3;
				// Give the solution buffer to gestalt-client for deserialization & storage
				gestalt_deserialize_solution(solution_buffer, solution_num_bytes);
			}
			else
			{
				solution_buffer[uart_rx_write_head++] = data[i];
			}
		}
		}
	}
}

int main(void)
{

	// initialize Kobuki library
	kobukiInit();

	// initialize RTT library
	NRF_LOG_INIT(NULL);
	NRF_LOG_DEFAULT_BACKENDS_INIT();
	printf("Initialized RTT!\n");

	// initialize i2c master (two wire interface)
	nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
	i2c_config.scl = BUCKLER_SENSORS_SCL;
	i2c_config.sda = BUCKLER_SENSORS_SDA;
	i2c_config.frequency = NRF_TWIM_FREQ_400K;
	ret_code_t error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
	APP_ERROR_CHECK(error_code);

	// initialize display
	nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
	nrf_drv_spi_config_t spi_config = {
		.sck_pin = BUCKLER_LCD_SCLK,
		.mosi_pin = BUCKLER_LCD_MOSI,
		.miso_pin = BUCKLER_LCD_MISO,
		.ss_pin = BUCKLER_LCD_CS,
		.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
		.orc = 0,
		.frequency = NRF_DRV_SPI_FREQ_4M,
		.mode = NRF_DRV_SPI_MODE_2,
		.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST};
	error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
	APP_ERROR_CHECK(error_code);
	display_init(&spi_instance);

	display_write("Goodbye, Human!", DISPLAY_LINE_0);
	printf("Display initialized!\n");

	nrf_delay_ms(2000);

	// initialize LSM9DS1 driver
	lsm9ds1_init(&twi_mngr_instance);
	printf("lsm9ds1 initialized\n");

	// initialize UART
	nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);

	while (1)
	{
		corapp_run();
		nrf_delay_ms(1);
	}
}
