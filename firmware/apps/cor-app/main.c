// cor-app
#include "cor-app.h"

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);


int main(void) {

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
	  .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
	};
	error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
	APP_ERROR_CHECK(error_code);
	display_init(&spi_instance);

	display_write("Hello, Human!", DISPLAY_LINE_0);
	printf("Display initialized!\n");

	nrf_delay_ms(2000);

	// initialize LSM9DS1 driver
	lsm9ds1_init(&twi_mngr_instance);
	printf("lsm9ds1 initialized\n");

	// initialize UART
	// Do not init serial, not enough memory
	//nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);

	while (1) {
		corapp_run();
		nrf_delay_ms(1);
	}
}

