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

#include "gestalt-client.h"

typedef enum {
  STOP,
  DRIVE,
  ALIGN_CW,
  ALIGN_CCW
} KobukiState_t;

void ser_event_handler(nrf_serial_t const* p_serial, nrf_serial_event_t event);
void ser_rx_data(uint8_t* data, size_t size);


void corapp_run();




