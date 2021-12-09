#include "ydlidar_x2.h"
#include "gestalt-client.h"

#include "nrf.h"
#include "nrf_uarte.h"
#include "nrf_serial.h"



typedef enum {
  STOP,
  DRIVE,
  ALIGN_CW,
  ALIGN_CCW,
  GRAB,
  RELEASE
} KobukiState_t;

void ser_event_handler(nrf_serial_t const* p_serial, nrf_serial_event_t event);
void ser_rx_data(uint8_t* data, size_t size);


void corapp_run();




