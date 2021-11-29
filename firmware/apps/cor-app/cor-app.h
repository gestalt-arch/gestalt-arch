#include "ydlidar_x2.h"

void ser_event_handler(nrf_serial_t const* p_serial, nrf_serial_event_t event);
void ser_rx_data(uint8_t* data, size_t size);

// After completing the serial read, deserialize the buffer into a Gestalt_path_stream_sol_t
void deserialize_solution_buffer();

void corapp_run();




