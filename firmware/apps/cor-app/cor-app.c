#include "cor-app.h"
#include <math.h>

#define DRIVE_SPEED 100
#define TURN_SPEED 100

// initialize state and state variables
KobukiState_t state = STOP;
KobukiSensors_t sensors = { 0 };
uint8_t timer = 0;
Gestalt_status_t status;
float cur_pos_error;
float cur_theta_error;
Gestalt_action_t action;

// UART over Virtual COM port
uint8_t uart_rx_state = 0; // 0 - nothing read 
						   // 1 - 'g''s' 
						   // 2 - num_bytes found
						   // 3 - done reading
uint16_t uart_rx_write_head = 0;
uint8_t solution_buffer[4096];
uint16_t solution_num_bytes;

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
	// packet footer: '\n'

	printf("%.*s\n", size, data); // print the received packets

	uint8_t fc_found = 0; // first char found flag
	uint8_t lc_found = 0; // last char found flag
	// TODO: if fc/lc are not within contiguous 16 bytes, we will not detect beginning or end
	// consider making these two vars global


	for (uint32_t i = 0; i < size; i++) {
		switch (uart_rx_state) {
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
			if (uart_rx_write_head + 4 == solution_num_bytes) { // write_head starts after first 4 bytes passed
																// TODO: check for \n here?
				uart_rx_state = 3;
				// Give the solution buffer to gestalt-client for deserialization & storage
				gestalt_deserialize_solution(solution_buffer, solution_num_bytes);
			}
			else {
				solution_buffer[uart_rx_write_head++] = data[i];
			}
		}
		}
	}
}

void corapp_run()
{
	// read sensors from robot
	kobukiSensorPoll(&sensors);

	status = gestalt_get_current_status();
	cur_pos_error = status.pos_error;
    cur_theta_error = status.theta_error;

    action = gestalt_get_current_action();

	// test current state
	switch (state) {
		case STOP:
			//	STOP to ALIGN_CW
			if (action == GESTALT_MOVE && cur_theta_error < 0) {
				state = ALIGN_CW;
			}
			// STOP to ALIGN_CCW
			else if (action == GESTALT_MOVE && cur_theta_error > 0) {
				state = ALIGN_CCW;
			}
			//	STOP to Grab
			else if (action == GESTALT_GRAB) {
				state = GRAB;
				timer = 0;
			} 
			//	STOP to Release
			else if (action == GESTALT_DROP) {
				state = RELEASE;
				timer = 0;
			}
			//	STOP
			else {
				kobukiDriveDirect(0, 0);
				gestalt_send_goal_complete();
			}
			break;
		case DRIVE:
			//	DRIVE to ALIGN_CW
			if (cur_theta_error <= -0.5) {
				state = ALIGN_CW;
			}
			//	DRIVE to ALIGN_CCW
			else if (cur_theta_error >= 0.5) {
				state = ALIGN_CCW;
			}
			//	DRIVE
			else if (cur_pos_error <= 0.1) {
				state = STOP;
				gestalt_send_goal_complete();
				kobukiDriveDirect(0, 0);
			}
			else {
				kobukiDriveDirect(DRIVE_SPEED, DRIVE_SPEED);
			}
			break;
		case ALIGN_CW:
			//	ALIGN_CW to DRIVE
			if (fabs(cur_theta_error) <= 0.5) {
				state = DRIVE;
			}
			//	ALIGN_CW
			else {
				kobukiDriveDirect(-DRIVE_SPEED, DRIVE_SPEED);
			}
			break;
		case ALIGN_CCW:
			// ALIGN_CCW to DRIVE
			if (fabs(cur_theta_error) <= 0.5) {
				state = DRIVE;
			}
			// ALIGN_CCW to ALIGN_CCW
			else {
				kobukiDriveDirect(DRIVE_SPEED, -DRIVE_SPEED);
			}
			break;
		case GRAB:
			//	GRAB to STOP
			if (timer >= 5) {
				state = STOP;
				gestalt_send_goal_complete();
				kobukiDriveDirect(0, 0);
			}
			//	GRAB
			else {
				//grab
				timer += 1;
			}
			break;
		case RELEASE:
			//	RELEASE to STOP
			if (timer >= 5) {
				state = STOP;
				gestalt_send_goal_complete();
				kobukiDriveDirect(0, 0);
			}
			// RELEASE
			else {
				//release
				timer += 1;
			}
			break;
	}
	// continue for 10 ms before checking state again
}