// cor-app
#include "cor-app.h"

#define GESTALT_BOT_ID 1

int main(void) {

	// initialize Kobuki library
	kobukiInit();
	kobukiDriveDirect(0,0);

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
	ret_code_t error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
	APP_ERROR_CHECK(error_code);
	display_init(&spi_instance);

	display_write("DISPLAY INIT", DISPLAY_LINE_0);

	// initialize cor-app
	// handles all peripheral init
	corapp_init();

	KobukiSensors_t sensors;
	kobukiSensorPoll(&sensors);

	// Initialize gestalt-client
	gestalt_init(GESTALT_BOT_ID, &sensors);

	while (1) {
		corapp_run();
		nrf_delay_ms(1);
	}
}

