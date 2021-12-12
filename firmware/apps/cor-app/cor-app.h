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
#include "simple_ble.h"

#include "buckler.h"
#include "display.h"

#include "kobukiActuator.h"
#include "kobukiSensorTypes.h"
#include "kobukiSensorPoll.h"
#include "kobukiUtilities.h"
#include "lsm9ds1.h"

#include "gestalt-client.h"

#define GESTALT_BOT_ID 1
#define BOT_BLE_ID 0xF0C1 // Set LSB = GESTALT_BOT_ID
#define BOT_BLE_NAME "GS 1" // Set name to "GS [BOT_ID]"

typedef enum {
  STOP,
  DRIVE,
  ALIGN_CW,
  ALIGN_CCW
} KobukiState_t;


void corapp_init();

void corapp_run();




