#include "cor-app.h"
#include <math.h>
#include <float.h>

#define DRIVE_SPEED 100
#define TURN_SPEED 50
#define ADJUST_SPEED 25
#define COLLISION_THRESHOLD 0.4f
#define FORCE_STOP_TIMER 500000
#define FORCE_REVERSE_TIMER 3000000
#define BOT_SYNC_TIMEOUT 5000000

// initialize state and state variables
static KobukiState_t state = STOP;
static KobukiSensors_t sensors = { 0 };
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
static uint32_t ble_timer_h;
static int8_t ble_state_change = 0;
static int8_t ble_rx_pending = 0;

// Collision prevention
static uint32_t force_stop_timer_h;
static bool force_stop_timeout_flag = false;
static KobukiStopType_t force_stop_type = KOBUKI_BUMP;

// Force reverse
static bool force_reverse_timeout_flag = false;


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

// 60267642-592e-11ec-bf63-0242ac130002
static simple_ble_service_t main_service = {{
  .uuid128 = {0x02,0x00,0x13,0xAC,0x42,0x02,0x63,0xBF,
              0xEC,0x11,0x2E,0x59,0x42,0x76,0x26,0x60}
}};

// 60267643-592e-11ec-bf63-0242ac130002
static simple_ble_char_t rx_char = {.uuid16 = 0x7643};
static char rx_value[228];
//static uint32_t rx_value;

// 60267644-592e-11ec-bf63-0242ac130002
static simple_ble_char_t tx_char = {.uuid16 = 0x7644};
static uint32_t tx_value;

void ble_evt_write(ble_evt_t const* p_ble_evt) {
  if (simple_ble_is_char_event(p_ble_evt, &rx_char)) {
    tx_value = 1;
  }
}


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

// Check the kobuki sensors for a bump collision
static inline bool check_bump_sensors(KobukiSensors_t* kobuki_sensors)
{
	return (kobuki_sensors->bumps_wheelDrops.bumpLeft || kobuki_sensors->bumps_wheelDrops.bumpCenter || kobuki_sensors->bumps_wheelDrops.bumpRight);
}

// Callback for advertising report
void ble_evt_adv_report(ble_evt_t const* p_ble_evt)
{
	ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
	//c0:98:e5:49:xx:xx
	//printf("\n\nReport received: %d %d\n", adv_report->peer_addr.addr[5], adv_report->peer_addr.addr[4]);
	if(adv_report->peer_addr.addr[1] == 0xF0 &&
		adv_report->peer_addr.addr[2] == 0x49 &&
		adv_report->peer_addr.addr[3] == 0xE5 &&
		adv_report->peer_addr.addr[4] == 0x98 &&
		adv_report->peer_addr.addr[5] == 0xC0 )
		{
			memcpy(ble_rx_buffer, adv_report->data.p_data, adv_report->data.len);
			ble_rx_pending = 1;
		}
}

// ble_timer_h virtual timer callback function
// handles switching between ble comm states (adv and scan) 
void ble_switch_state_evt()
{
	if(ble_state_change != 1) {
		ble_comm_state = (ble_comm_state == 1) ? 0 : 1;
		printf("*BLE state switch to %d*\n", ble_comm_state);
		ble_state_change = 1;
	}

	// refresh comm interval timer
	uint32_t ble_comm_interval = get_random_comm_interval(BLE_COMM_INT_L, BLE_COMM_INT_H);
	ble_timer_h = virtual_timer_start(ble_comm_interval, &ble_switch_state_evt);
}

void handle_ble_state_change()
{
	printf("BLE> State change %d\n", ble_comm_state);
	if(ble_comm_state == 1) {
		// transition to scan state
		advertising_stop();
		scanning_start();
		printf("BLE> Scanning started\n");
	}
	else {
		scanning_stop();
		printf("BLE> Scanning stopped\n");
	}
	ble_state_change = 0;
}

static void collision_timer_evt()
{
	force_stop_timeout_flag = true;
}

static void force_reverse_timer_evt()
{
	force_reverse_timeout_flag = true;
}


// Generate a temporary avoidance goal that turns to the right X degrees and travels
// a random (bounded) distance in that direction.
static Gestalt_goal_t generate_avoid_goal(const Gestalt_vector2_t* curr_pos, float curr_theta)
{
	float dist = (float)((rand() % (300 - 100 + 1)) + 100) / 1000.f;
	float theta = curr_theta + 45.f;
	Gestalt_vector2_t new_pos = *curr_pos;
	gestalt_transform_vector(&new_pos, dist, theta);

	Gestalt_goal_t avoid_goal;
	avoid_goal.curr_action_goal = GESTALT_MOVE;
	avoid_goal.curr_x_goal = new_pos.x;
	avoid_goal.curr_y_goal = new_pos.y;
	return avoid_goal;
}

// Check collisions with other bots
int8_t corapp_check_bot_collisions(Gestalt_vector2_t curr_pos, Gestalt_bot_status_t* b_list)
{
	for(int i = 0; i < MAX_BOTS; i++)
	{
		if((i+1) != GESTALT_BOT_ID  && b_list[i].valid && (gestalt_timer_read(COMM_TIMER) - b_list[i].last_sync_time) < BOT_SYNC_TIMEOUT){
			float dist = gestalt_get_2d_dist(curr_pos.x, curr_pos.y, b_list[i].x, b_list[i].y);
			if(dist < COLLISION_THRESHOLD)
				return b_list[i].bot_id;
		}
	}
	return -1;
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

	//nrf_gpio_cfg_output(BUCKLER_LED0);

  	nrf_delay_ms(1);

	char buf[16];
	sprintf(buf, "device id %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	display_write(buf, DISPLAY_LINE_0);

	tx_value = 0;



	// Setup BLE
	simple_ble_app = simple_ble_init(&ble_config);
	sprintf(buf, "2 %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	display_write(buf, DISPLAY_LINE_0);
	simple_ble_add_service(&main_service);
	sprintf(buf, "3 %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	display_write(buf, DISPLAY_LINE_0);
	simple_ble_add_characteristic(1, 1, 0, 0, sizeof(rx_value), (char*)&rx_value, &main_service, &rx_char);
	simple_ble_add_characteristic(1, 1, 0, 0, sizeof(tx_value), (uint8_t*)&tx_value, &main_service, &tx_char);

	nrf_delay_ms(50);
	// Start Advertising
	simple_ble_adv_only_name();


	sprintf(buf, "4 %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	printf("4");
	display_write(buf, DISPLAY_LINE_0);
	while(tx_value != 1) {
		power_manage();
	}
	nrf_delay_ms(2000);
	printf(rx_value);
	//handle_ble_state_change();
	advertising_stop();

	nrf_delay_ms(2000);
	sprintf(buf, "5 %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	printf("5");
	display_write(buf, DISPLAY_LINE_0);
	// Init BLE
	//simple_ble_app = simple_ble_init(&ble_config);
	ble_comm_state = 0; // start in broadcast mode
	// start switch timer
	uint32_t ble_comm_interval = get_random_comm_interval(BLE_COMM_INT_L, BLE_COMM_INT_H);
	ble_timer_h = virtual_timer_start(ble_comm_interval, &ble_switch_state_evt);
	for(int i = 0; i < BLE_BUFF_SIZE; i++)
		ble_tx_buffer[i] = 0;
	printf("9");
	simple_ble_adv_manuf_data(ble_tx_buffer, BLE_BUFF_SIZE);
	sprintf(buf, "6 %x:%x", ble_config.device_id >> 8, ble_config.device_id & 0xFF);
	printf("6");
	display_write(buf, DISPLAY_LINE_0);
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
	if(ble_state_change == 1) 
		handle_ble_state_change();

	if(ble_comm_state == 0) {
		// transition to scan state
		gestalt_prep_ble_buffer(ble_tx_buffer);
		//for(int i = 0; i < 14; i++ )
		//	printf("TX Byte [%d]: %x\n", i, ble_tx_buffer[i]);
		simple_ble_adv_manuf_data(ble_tx_buffer, BLE_BUFF_SIZE);
	}

	if(ble_rx_pending == 1) {
		gestalt_parse_ble_buffer(ble_rx_buffer, 31);
		ble_rx_pending = 0;
	}

	Gestalt_bot_status_t* b_list = gestalt_get_status_list();

	int8_t collide_bot = corapp_check_bot_collisions(status->curr_pos, b_list);
	//printf("pos (o): %1.2f, %1.2f", 
	//	b_list[3].x, b_list[3].y);
	//display_write(disp_buffer, DISPLAY_LINE_0);

    action = gestalt_get_current_action();

	// test current state
	switch (state) {
		case STOP: {
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
			}
			break;
		}
		case DRIVE: {
			// DRIVE to FORCE_STOP - BUMP type
			if(check_bump_sensors(&sensors)) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_BUMP;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			// DRIVE to FORCE_STOP - GESTALT type
			else if(collide_bot > -1) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_GESTALT;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			//	DRIVE to ALIGN_CW
			else if (cur_theta_error <= -3.f) {
				state = ALIGN_CCW;
				turn_speed = ADJUST_SPEED;
			}
			//	DRIVE to ALIGN_CCW
			else if (cur_theta_error >= 3.0f) {
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
		}
		case ALIGN_CW: {
			if(check_bump_sensors(&sensors)) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_BUMP;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			// DRIVE to FORCE_STOP - GESTALT type
			else if(collide_bot > -1) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_GESTALT;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			//	ALIGN_CW to DRIVE
			else if (fabs(cur_theta_error) <= 0.5) {
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
		}
		case ALIGN_CCW: { 
			// ALIGN_CCW to DRIVE
			if(check_bump_sensors(&sensors)) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_BUMP;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			// DRIVE to FORCE_STOP - GESTALT type
			else if(collide_bot > -1) {
				state = FORCE_STOP;
				force_stop_type = KOBUKI_GESTALT;
				kobukiDriveDirect(0,0);
				force_stop_timer_h = virtual_timer_start(FORCE_STOP_TIMER, &collision_timer_evt);
				force_stop_timeout_flag = false;
			}
			else if (fabs(cur_theta_error) <= 0.5f) {
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
		case FORCE_STOP: {
			if(force_stop_timeout_flag){
				force_stop_timeout_flag = false;
				state = FORCE_REVERSE;
				virtual_timer_start(FORCE_REVERSE_TIMER, &force_reverse_timer_evt);
				kobukiDriveDirect(-DRIVE_SPEED, -DRIVE_SPEED);
			}
			else {
				display_write("FORCE_STOP", DISPLAY_LINE_0);
				kobukiDriveDirect(0, 0);
			}
			break;
		}
		case FORCE_REVERSE: {
			if(force_reverse_timeout_flag) {
				force_reverse_timeout_flag = false;
				Gestalt_goal_t tmp_goal = generate_avoid_goal(&status->curr_pos, status->curr_theta);
				gestalt_force_goal(&tmp_goal);
				state = STOP;
				kobukiDriveDirect(0, 0);
			}
			else {
				display_write("FORCE_REVERSE", DISPLAY_LINE_0);
				kobukiDriveDirect(-DRIVE_SPEED, -DRIVE_SPEED);
			}
			break;
		}

	}
	// continue for 1 ms before checking state again
}