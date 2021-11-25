#include "cor-app.h"

// initialize state
KobukiState_t state = OFF;
KobukiSensors_t sensors = { 0 };

void ser_event_handler(nrf_serial_t const* p_serial, nrf_serial_event_t event)
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

void ser_rx_data(uint8_t* data, size_t size) {
	// Do something useful with recieved data
	printf("%.*s", size, data);
}


void corapp_run()
{
	// read sensors from robot
	kobukiSensorPoll(&sensors);


	// test current state
	switch (state) {
		// TODO
	};

	// continue for 10 ms before checking state again
}