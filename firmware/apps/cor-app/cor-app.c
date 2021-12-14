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

// BLE scan/adv state flag
static int8_t ble_comm_state = 0;
uint32_t ble_timer_handle;

#define BLE_COMM_INT_L 200000  // lower bound comm interval (us)
#define BLE_COMM_INT_H 1000000 // upper bound comm interval (us)


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

// get random uint32_t interval between lower bound (l_bound) and upper bound (u_bound)
// to be used for timing the ble comm interval switching
static inline uint32_t get_random_comm_interval(uint32_t l_bound, uint32_t u_bound)
{
	return (rand() % (u_bound - l_bound + 1)) + l_bound;
}

// Callback for advertising report
void ble_evt_adv_report(ble_evt_t const* p_ble_evt)
{
	ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
	//c0:98:e5:49:xx:xx
	printf("\n\nReport received: %d %d\n", adv_report->peer_addr.addr[5], adv_report->peer_addr.addr[4]);
	if(adv_report->peer_addr.addr[5] == 0xC0)
		printf("FILTERED Report received: %x\n", adv_report->peer_addr.addr);
}

// ble_timer_handle virtual timer callback function
// handles switching between ble comm states (adv and scan) 
void ble_switch_state()
{
	printf("*BLE state switch*\n");
	ble_comm_state = (ble_comm_state == 1) ? 0 : 1;

	// refresh comm interval timer
	uint32_t ble_comm_interval = get_random_comm_interval(BLE_COMM_INT_L, BLE_COMM_INT_H);
	ble_timer_handle = virtual_timer_start(ble_comm_interval, &ble_switch_state);
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

	nrf_delay_ms(2000);

	// Init virtual timer library
	virtual_timer_init();

	// Init BLE
	simple_ble_app = simple_ble_init(&ble_config);
	ble_comm_state = 0; // start in broadcast mode
	// start switch timer
	uint32_t ble_comm_interval = get_random_comm_interval(BLE_COMM_INT_L, BLE_COMM_INT_H);
	ble_timer_handle = virtual_timer_start(ble_comm_interval, &ble_switch_state);
	for(int i = 0; i < BLE_BUFF_SIZE; i++)
		ble_tx_buffer[i] = 0;
	simple_ble_adv_manuf_data(ble_tx_buffer, BLE_BUFF_SIZE);

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
	// ONLY if in advertise comm state
	if(ble_comm_state == 1) {
		// transition to scan state
		advertising_stop();
		scanning_start();
	}
	else {
		scanning_stop();
		simple_ble_adv_manuf_data(ble_tx_buffer, BLE_BUFF_SIZE);
	}

	//printf("curr_theta = %1.3f | cur_theta_error: %1.3f\n", status->curr_theta, cur_theta_error);
    action = gestalt_get_current_action();

	// test current state
	switch (state) {
		case STOP:
			//	STOP to ALIGN_CW
			if (action == GESTALT_MOVE && cur_theta_error < -FLT_EPSILON) {
				//state = ALIGN_CCW;
				turn_speed = TURN_SPEED;
			}
			// STOP to ALIGN_CCW
			else if (action == GESTALT_MOVE && cur_theta_error > FLT_EPSILON) {
				//state = ALIGN_CW;
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