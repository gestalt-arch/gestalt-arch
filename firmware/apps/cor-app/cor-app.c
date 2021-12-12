#include "cor-app.h"
#include <math.h>
#include <float.h>

#define DRIVE_SPEED 100
#define TURN_SPEED 50
#define ADJUST_SPEED 25

// initialize state and state variables
static KobukiState_t state = STOP;
static KobukiSensors_t sensors = { 0 };
static uint8_t timer = 0;
static Gestalt_status_t* status;
static float cur_pos_error;
static float cur_theta_error;
static Gestalt_action_t action;
// LCD display buffer
char disp_buffer[32];

// BLE send & receive buffers
uint8_t ble_rx_buffer[32];
uint8_t ble_tx_buffer[32];
#define BLE_BUFF_SIZE 14

static int8_t turn_speed = TURN_SPEED;

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = BOT_BLE_ID, // TODO: replace with your lab bench number
        .adv_name          = BOT_BLE_NAME, // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

simple_ble_app_t* simple_ble_app;


#define I2C_DEVICE_ID 0x66

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

static void i2c_read_bytes(uint8_t i2c_addr, uint8_t reg_addr, uint8_t* data, uint8_t len) {
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, data, len, 0),
  };
  ret_code_t error_code = nrf_twi_mngr_perform(&twi_mngr_instance, NULL, read_transfer, 2, NULL);
  APP_ERROR_CHECK(error_code);
}

static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0),
  };
  ret_code_t error_code = nrf_twi_mngr_perform(&twi_mngr_instance, NULL, read_transfer, 2, NULL);
  APP_ERROR_CHECK(error_code);
  return rx_buf;
}

static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, buf, 2, 0),
  };
  ret_code_t error_code = nrf_twi_mngr_perform(&twi_mngr_instance, NULL, write_transfer, 1, NULL);
  APP_ERROR_CHECK(error_code);
}


void corapp_init()
{
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

	// Init BLE
	simple_ble_app = simple_ble_init(&ble_config);

	nrf_delay_ms(2000);

	// initialize LSM9DS1 driver
	lsm9ds1_init(&twi_mngr_instance);
	printf("lsm9ds1 initialized\n");
}

void corapp_run()
{
	// read sensors from robot
	kobukiSensorPoll(&sensors);
	
	// update gestalt-client
	gestalt_update_sensor_data(&sensors);

	// read back the status from gestalt
	status = gestalt_get_current_status();
	cur_pos_error = status->pos_error;
    cur_theta_error = status->theta_error;

	// update BLE advertise buffer
	gestalt_prep_ble_buffer(ble_tx_buffer); // should be full of zeros
	simple_ble_adv_manuf_data(ble_tx_buffer, BLE_BUFF_SIZE);

	printf("curr_theta = %1.3f | cur_theta_error: %1.3f\n", status->curr_theta, cur_theta_error);
    action = gestalt_get_current_action();

	// test current state
	switch (state) {
		case STOP:
			//	STOP to ALIGN_CW
			if (action == GESTALT_MOVE && cur_theta_error < -FLT_EPSILON) {
				state = ALIGN_CCW;
				turn_speed = TURN_SPEED;
			}
			// STOP to ALIGN_CCW
			else if (action == GESTALT_MOVE && cur_theta_error > FLT_EPSILON) {
				state = ALIGN_CW;
				turn_speed = TURN_SPEED;
			}
			// STOP to DRIVE
			else if(action == GESTALT_MOVE && cur_theta_error >= -FLT_EPSILON && cur_theta_error <= FLT_EPSILON) {
				state = DRIVE;
			}
			//	STOP
			else {
				display_write("STOP", DISPLAY_LINE_0);
				kobukiDriveDirect(0, 0);
				gestalt_send_goal_complete();
			}
			break;
		case DRIVE:
			//	DRIVE to ALIGN_CW
			if (cur_theta_error <= -1.f) {
				state = ALIGN_CCW;
				turn_speed = ADJUST_SPEED;
			}
			//	DRIVE to ALIGN_CCW
			else if (cur_theta_error >= 1.0f) {
				state = ALIGN_CW;
				turn_speed = ADJUST_SPEED;
			}
			//	DRIVE
			else if (cur_pos_error <= 0.01f) {
				state = STOP;
				gestalt_send_goal_complete();
				kobukiDriveDirect(0, 0);
			}
			else {
				display_write("DRIVE", DISPLAY_LINE_0);
				kobukiDriveDirect(DRIVE_SPEED, DRIVE_SPEED);
				sprintf(disp_buffer, "pos: %1.2f, %1.2f", 
					status->curr_pos.x, status->curr_pos.y);
				display_write(disp_buffer, DISPLAY_LINE_1);
			}
			break;
		case ALIGN_CW:
			//	ALIGN_CW to DRIVE
			if (fabs(cur_theta_error) <= 0.5) {
				state = DRIVE;
			}
			//	ALIGN_CW
			else {
				display_write("ALIGN CW", DISPLAY_LINE_0);
				kobukiDriveDirect(turn_speed, -turn_speed);
				sprintf(disp_buffer, "%1.2f, %1.2f", status->curr_theta, cur_theta_error);
				display_write(disp_buffer, DISPLAY_LINE_1);
			}
			break;
		case ALIGN_CCW:
			// ALIGN_CCW to DRIVE
			if (fabs(cur_theta_error) <= 0.5f) {
				state = DRIVE;
			}
			// ALIGN_CCW to ALIGN_CCW
			else {
				display_write("ALIGN CCW", DISPLAY_LINE_0);
				kobukiDriveDirect(-turn_speed, turn_speed);
				sprintf(disp_buffer, "%1.2f, %1.2f", status->curr_theta, cur_theta_error);
				display_write(disp_buffer, DISPLAY_LINE_1);
			}
			break;
	}
	// continue for 1 ms before checking state again
}