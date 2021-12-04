#include "cor-app.h"

// initialize state
KobukiState_t state = OFF;
KobukiSensors_t sensors = { 0 };


// UART over Virtual COM port
uint8_t uart_rx_flag = 0;
uint16_t uart_rx_write_head = 0;
uint8_t solution_buffer[4096];

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
	// Determine if path-stream solution being transmitted
	// packet header: 'g' 's'
	// packet footer: '\r' '\n'

	printf("%.*s", size, data); // print the received packets

	uint8_t fc_found = 0; // first char found flag
	uint8_t lc_found = 0; // last char found flag
	// TODO: if fc/lc are not within contiguous 16 bytes, we will not detect beginning or end
	// consider making these two vars global
	for (uint32_t i = 0; i < size; i++) {
		if (uart_rx_flag == 0) {
			if (data[i] == 'g')
				fc_found = 1; // found first g
			else if (fc_found == 1 && data[i] == 's')
				uart_rx_flag = 1; // found 'g' then 's'
			else if (fc_found == 1 && data[i] != 's')
				fc_found = 0;
			else
				// do nothing
		}
		else if (uart_rx_flag == 1) {
			if (data[i] == '\r')
				lc_found = 1;
			else if (lc_found == 1 && data[i] == '\n') {
				uart_rx_flag = 2;
			}
			else if (lc_found == 1 && data[i] != '\n')
				lc_found = 0;
			else
				solution_buffer[uart_rx_write_head++] = data[i];
		}
	}
	
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