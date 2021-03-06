#include "gestalt-client.h"
#include "string.h"
#include "nrf.h"

// The complete path stream solution
static Gestalt_path_stream_sol_t ps_solution;

// The path stream for this bot to follow
// determined after gestalt_init and solution deserialization
static Gestalt_path_stream_t target_ps;
static uint8_t _this_id;

// Contains all info regarding the current status of the robot
static Gestalt_status_t curr_status;

// Contains the goal position and action of the bot
static Gestalt_goal_t curr_goal;
static bool forced_goal = false;

// Keep track of other bots
// Updated by BLE communication
static Gestalt_bot_status_t bot_status_list[MAX_BOTS];

// Keep history of encoder values
static uint16_t prev_encoder_left;
static uint16_t prev_encoder_right;

#define STRAIGHT_DIST_THRESHOLD 0.05
#define ANGLE_TICK_TO_DEG 0.00875
#define BOT_WHEEL_RADIUS 35.f   // bot wheel radius (in mm)
#define BOT_WHEEL_BASE 0.230f   // bot wheel base (in m)

#define BOT_LOSS_TIMEOUT 5000000 // 5s timeout determines bot is lost

float gestalt_get_2d_dist(float x1, float y1, float x2, float y2)
{
	return sqrtf(powf(x2-x1, 2) + powf(y2-y1, 2));
}

// Returns the provided vector translated by a distance in direction theta
void gestalt_transform_vector(Gestalt_vector2_t* vec, float dist, float theta)
{
	float theta_rad = theta * (M_PI / 180.f);
	// tokyo drift
	vec->x += dist * sinf(theta_rad);
	vec->y += dist * cosf(theta_rad);
}

inline static float get_2d_theta(float x1, float y1, float x2, float y2)
{
	return (90.f - (atan2f(y2 - y1, x2 - x1) * (180.f / M_PI)));
}

inline static float get_encoder_dist(uint16_t curr_encoder, uint16_t prev_encoder) 
{
	const float CONVERSION = 0.000085292090497737556558f;
	float dist;
	dist = curr_encoder - prev_encoder;
	return dist * CONVERSION;
}

// Provide distance traveled by each wheel (calculated using get_encoder_dist)
// returns calculated theta as 360 * (wheel dist / wheel turning circumference)
// returns 0 if both wheels moving in same direction
static float get_encoder_rotation(float left_dist, float right_dist)
{
	if(left_dist > 0 && right_dist < 0)
		return 360.f*(((fabs(right_dist) + left_dist) / 2.f) / (BOT_WHEEL_BASE * M_PI));
	else if(left_dist < 0 && right_dist > 0)
		return -1*(360.f*(((right_dist + fabs(left_dist)) / 2.f) / (BOT_WHEEL_BASE * M_PI)));
	else {
		return 360.f*(((left_dist - right_dist) / (BOT_WHEEL_BASE * M_PI)));
	}
}

inline static void update_errors()
{
	float dist = gestalt_get_2d_dist(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.pos_error = dist;

	float theta_target = get_2d_theta(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.theta_error = theta_target - curr_status.curr_theta;

	if(curr_status.theta_error < -180.f)
		curr_status.theta_error += 360.f;
	else if(curr_status.theta_error > 180.f) 
		curr_status.theta_error += -360.f;
}

static void deserialize_path_stream(uint8_t* path_stream, uint32_t path_length, uint8_t ps_idx) 
{
	uint32_t r_ptr = 0;
	uint32_t tmp_word;

	ps_solution.path_stream_vector[ps_idx].path_length = path_length;

	ps_solution.path_stream_vector[ps_idx].bot_id = (uint8_t)path_stream[r_ptr++];
	// x_pos_stream
	for (uint32_t i = 0; i < path_length; i ++) {
		tmp_word = (path_stream[r_ptr] |
			path_stream[r_ptr + 1] << 8 |
			path_stream[r_ptr + 2] << 16 |
			path_stream[r_ptr + 3] << 24);
		ps_solution.path_stream_vector[ps_idx].x_pos_stream[i] = (
			*((float*)&tmp_word)
			);
		r_ptr += 4;
	}
	// y_pos_stream
	for (uint32_t i = 0; i < path_length; i++) {
		tmp_word = (path_stream[r_ptr] |
			path_stream[r_ptr + 1] << 8 |
			path_stream[r_ptr + 2] << 16 |
			path_stream[r_ptr + 3] << 24);
		ps_solution.path_stream_vector[ps_idx].y_pos_stream[i] = (
			*((float*)&tmp_word)
			);
		r_ptr += 4;
	}
	// action_stream
	for (uint32_t i = 0; i < path_length; i++) {
		tmp_word = (path_stream[r_ptr] |
			path_stream[r_ptr + 1] << 8 |
			path_stream[r_ptr + 2] << 16 |
			path_stream[r_ptr + 3] << 24);
		ps_solution.path_stream_vector[ps_idx].action_stream[i] = (int32_t)tmp_word;
		r_ptr += 4;
	}
	// exclusion_stream
	for (uint32_t i = 0; i < path_length; i++) {
		tmp_word = (path_stream[r_ptr] |
			path_stream[r_ptr + 1] << 8 |
			path_stream[r_ptr + 2] << 16 |
			path_stream[r_ptr + 3] << 24);
		ps_solution.path_stream_vector[ps_idx].exclusion_stream[i] = (int32_t)tmp_word;
		r_ptr += 4;
	}
}

// After completing the serial read, deserialize the buffer into a Gestalt_path_stream_sol_t
void gestalt_deserialize_solution(uint8_t* solution_buffer, uint16_t solution_num_bytes)
{
	uint32_t read_ptr = 0;
	ps_solution.num_path_streams = solution_buffer[read_ptr++];

	uint8_t path_length;
	for (uint8_t i = 0; i < ps_solution.num_path_streams; i++) {
		path_length = solution_buffer[read_ptr++];
		deserialize_path_stream((solution_buffer+read_ptr), path_length, i);
		read_ptr += path_length + 1;
	}
}

// For debugging, hardcode a test path as a series of waypoints
void gestalt_init_test_path() 
{
	ps_solution.num_path_streams = 3;

	// BOT 1
	ps_solution.path_stream_vector[0].bot_id = 1;
	ps_solution.path_stream_vector[0].path_length = 4;
	
	ps_solution.path_stream_vector[0].x_pos_stream[0] = 0.0f;
	ps_solution.path_stream_vector[0].y_pos_stream[0] = 0.0f;
	ps_solution.path_stream_vector[0].action_stream[0] = GESTALT_MOVE;

	ps_solution.path_stream_vector[0].x_pos_stream[1] = 1.f;
	ps_solution.path_stream_vector[0].y_pos_stream[1] = 1.f;
	ps_solution.path_stream_vector[0].action_stream[1] = GESTALT_MOVE;

	ps_solution.path_stream_vector[0].x_pos_stream[2] = 0.0f;
	ps_solution.path_stream_vector[0].y_pos_stream[2] = 1.f;
	ps_solution.path_stream_vector[0].action_stream[2] = GESTALT_MOVE;

	ps_solution.path_stream_vector[0].x_pos_stream[3] = 0.0f;
	ps_solution.path_stream_vector[0].y_pos_stream[3] = 0.0f;
	ps_solution.path_stream_vector[0].action_stream[3] = GESTALT_MOVE;

	// BOT 2
	ps_solution.path_stream_vector[1].bot_id = 2;
	ps_solution.path_stream_vector[1].path_length = 4;

	ps_solution.path_stream_vector[1].x_pos_stream[0] = 0.0f;
	ps_solution.path_stream_vector[1].y_pos_stream[0] = 0.0f;
	ps_solution.path_stream_vector[1].action_stream[0] = GESTALT_MOVE;

	ps_solution.path_stream_vector[1].x_pos_stream[1] = -0.5f;
	ps_solution.path_stream_vector[1].y_pos_stream[1] = 0.5f;
	ps_solution.path_stream_vector[1].action_stream[1] = GESTALT_MOVE;

	ps_solution.path_stream_vector[1].x_pos_stream[2] = 0.5f;
	ps_solution.path_stream_vector[1].y_pos_stream[2] = -0.5f;
	ps_solution.path_stream_vector[1].action_stream[2] = GESTALT_MOVE;

	ps_solution.path_stream_vector[1].x_pos_stream[3] = 0.0f;
	ps_solution.path_stream_vector[1].y_pos_stream[3] = 0.0f;
	ps_solution.path_stream_vector[1].action_stream[3] = GESTALT_MOVE;

	// BOT 3
	ps_solution.path_stream_vector[2].bot_id = 3;
	ps_solution.path_stream_vector[2].path_length = 4;
	
	ps_solution.path_stream_vector[2].x_pos_stream[0] = 0.5f;
	ps_solution.path_stream_vector[2].y_pos_stream[0] = 0.0f;
	ps_solution.path_stream_vector[2].action_stream[0] = GESTALT_MOVE;

	ps_solution.path_stream_vector[2].x_pos_stream[1] = 0.5f;
	ps_solution.path_stream_vector[2].y_pos_stream[1] = 1.0f;
	ps_solution.path_stream_vector[2].action_stream[1] = GESTALT_MOVE;

	ps_solution.path_stream_vector[2].x_pos_stream[2] = 1.f;
	ps_solution.path_stream_vector[2].y_pos_stream[2] = 1.f;
	ps_solution.path_stream_vector[2].action_stream[2] = GESTALT_MOVE;

	ps_solution.path_stream_vector[2].x_pos_stream[3] = 0.5f;
	ps_solution.path_stream_vector[2].y_pos_stream[3] = 0.0f;
	ps_solution.path_stream_vector[2].action_stream[3] = GESTALT_MOVE;

}


// Initialize the gestalt client
// Must be called AFTER providing the deserialized solution via gestalt_deserialize_solution
// Must be called BEFORE calling any other gestalt client functions
// 
// Provide the bot id
void gestalt_init(uint8_t bot_id, KobukiSensors_t* kobuki_sensors) 
{
	_this_id = bot_id;
	// timer for sensor integration
	gestalt_timer_init(SENSOR_TIMER);
	// timer for bot sync tracking
	gestalt_timer_init(COMM_TIMER);

	gestalt_init_test_path(); // debug only

	// find path assigned to this bot
	// should probably search in the future
	target_ps = ps_solution.path_stream_vector[_this_id-1];
	printf("My BOT ID: %d", _this_id);

	// initialize current position/theta, ID, and progress
	curr_status.curr_pos.x = target_ps.x_pos_stream[0];
	curr_status.curr_pos.y = target_ps.y_pos_stream[0];
	curr_status.curr_theta = 0.f;
	curr_status.bot_id = bot_id;
	curr_status.ps_progress = -1;

	// init goals
	curr_goal.curr_x_goal = target_ps.x_pos_stream[1];
	curr_goal.curr_y_goal = target_ps.y_pos_stream[1];
	curr_goal.curr_action_goal = target_ps.action_stream[1];

	// save current encoder values
	prev_encoder_left = kobuki_sensors->leftWheelEncoder;
	prev_encoder_right = kobuki_sensors->rightWheelEncoder;

	// init bot status list
	for(int i = 0; i < MAX_BOTS; i++)
	{
		bot_status_list[i].x = 0.f;
		bot_status_list[i].y = 0.f;
		bot_status_list[i].theta = 0.f;
		bot_status_list[i].ps_progress = -1;
		bot_status_list[i].bot_id = i + 1;
		bot_status_list[i].valid = 0;
	}

	update_errors();
	gestalt_send_goal_complete();
	gestalt_timer_reset(SENSOR_TIMER);
	gestalt_timer_reset(COMM_TIMER);
}

// Update the sensor data and all internal state space representations
void gestalt_update_sensor_data(KobukiSensors_t* kobuki_sensors)
{
	float dist_left = get_encoder_dist(kobuki_sensors->leftWheelEncoder, prev_encoder_left);
	float dist_right = get_encoder_dist(kobuki_sensors->rightWheelEncoder, prev_encoder_right);

	float dist_diff, dist;
	dist_diff = fabs(dist_left - dist_right);

	float encoder_theta = get_encoder_rotation(dist_left, dist_right);

	// only increment distance if the wheels were traveling approximately straight
	dist = (dist_diff <= STRAIGHT_DIST_THRESHOLD) ? ((dist_left + dist_right) / 2) : 0.f;
	// Filter unreasonable distances
	dist = (dist > 1.f) ? 0.f : dist;

	// save current encoder values
	prev_encoder_left = kobuki_sensors->leftWheelEncoder;
	prev_encoder_right = kobuki_sensors->rightWheelEncoder;

	float angle_rate = ((float)(-1*kobuki_sensors->angleRate) * 0.00875f);
	// Get cycle time passed (this function must be called once per main() while interation)
	float delta_t = ((float)gestalt_timer_read(3) / 1000000.f);
	//printf("Time passed: %1.5f\n", delta_t);
	
	// attempt to integrate theta
	float gyro_theta = (angle_rate * delta_t); 

	float theta = (fabs(encoder_theta) > 1.f) ? gyro_theta : encoder_theta;

	// update current theta
	curr_status.curr_theta += theta;
	curr_status.curr_theta = fmod(curr_status.curr_theta, 360.f);

	gestalt_transform_vector(&curr_status.curr_pos, dist, curr_status.curr_theta);

	//printf("left dist: %1.4f\tright dist: %1.4f\n", dist_left, dist_right);
	//printf("enc theta: %1.4f\tgyro theta: %1.4f\n", encoder_theta, gyro_theta);

	update_errors();
	gestalt_timer_reset(3);
}

// Append a lost bot's pathstream (whatever was left)
static void append_bot_pathstream(uint8_t bot_id) {
	// 1 - 2 3 4
	// 5 6 - 7 8
	// 1 - 2 7 3 4
	int16_t copy_length = ps_solution.path_stream_vector[bot_id-1].path_length - bot_status_list[bot_id-1].ps_progress - 1;
	if(copy_length <= 0) 
		return;
	// x
	// shift out of the way
	uint8_t* dest = (target_ps.x_pos_stream + curr_status.ps_progress + 1 + copy_length);
	uint8_t* src = target_ps.x_pos_stream + curr_status.ps_progress + 1;
	uint32_t len = target_ps.path_length - curr_status - 1;
	memcpy(dest, src, len);
	// append
	dest = target_ps.x_pos_stream + curr_status.ps_progress + 1;
	src = ps_solution.path_stream_vector[bot_id-1].x_pos_stream + bot_status_list[bot_id-1].ps_progress;
	memcpy(dest, src, copy_length);

	// y
	// shift out of the way
	dest = (target_ps.y_pos_stream + curr_status.ps_progress + 1 + copy_length);
	src = target_ps.y_pos_stream + curr_status.ps_progress + 1;
	len = target_ps.path_length - curr_status - 1;
	memcpy(dest, src, len);
	// append
	dest = target_ps.y_pos_stream + curr_status.ps_progress + 1;
	src = ps_solution.path_stream_vector[bot_id-1].y_pos_stream + bot_status_list[bot_id-1].ps_progress;
	memcpy(dest, src, copy_length);

	// action
	// shift out of the way
	dest = (target_ps.action_stream + curr_status.ps_progress + 1 + copy_length);
	src = target_ps.action_stream + curr_status.ps_progress + 1;
	len = target_ps.path_length - curr_status - 1;
	memcpy(dest, src, len);
	// append
	dest = target_ps.action_stream + curr_status.ps_progress + 1;
	src = ps_solution.path_stream_vector[bot_id-1].action_stream + bot_status_list[bot_id-1].ps_progress;
	memcpy(dest, src, copy_length);

	// exclusion
	// shift out of the way
	dest = (target_ps.exclusion_stream + curr_status.ps_progress + 1 + copy_length);
	src = target_ps.exclusion_stream + curr_status.ps_progress + 1;
	len = target_ps.path_length - curr_status - 1;
	memcpy(dest, src, len);
	// append
	dest = target_ps.exclusion_stream + curr_status.ps_progress + 1;
	src = ps_solution.path_stream_vector[bot_id-1].exclusion_stream + bot_status_list[bot_id-1].ps_progress;
	memcpy(dest, src, copy_length);
}

void gestalt_update_bot_status()
{
	for(int i = 0; i < MAX_BOTS; i++) 
	{
		float tx_time_elapsed = (gestalt_timer_read(COMM_TIMER) - bot_status_list[i].last_sync_time);

		if((i+1) != _this_id && bot_status_list[i].valid &&  tx_time_elapsed < BOT_LOSS_TIMEOUT ) {
			// mark no long valid
			bot_status_list[i].valid = 0;

			// Take over the path stream?
			int8_t min = 127;
			uint8_t minIdx = 1;

			for(int i = 0; i < MAX_BOTS; i++) {
				if(bot_status_list[i].valid || i == (_this_id - 1)){
					// e.g. 1 takes over 2 instead of 3
					// 2 takes over 1 instead of 3
					int8_t diff = _this_id - bot_status_list[i].bot_id;
					if(diff < min) {
						min = diff;
						minIdx = (i+1);
					}
				}
			}
			// Am I optimal choice to take over path stream?
			if(minIdx == _this_id) {
				append_bot_pathstream(bot_status_list[i].bot_id);
			}
		}
	}
}


// Inform gestalt client that the active goal is complete
void gestalt_send_goal_complete()
{
	if(!forced_goal)
	{
		// increment ps progress
		if(curr_status.ps_progress < target_ps.path_length - 1)
			curr_status.ps_progress += 1;	
	}
	// update goals
	uint8_t ps_prog = curr_status.ps_progress;
	uint8_t i = (curr_status.ps_progress >= target_ps.path_length - 1) ? ps_prog : ps_prog + 1;
	curr_goal.curr_x_goal = target_ps.x_pos_stream[i];
	curr_goal.curr_y_goal = target_ps.y_pos_stream[i];
	curr_goal.curr_action_goal = target_ps.action_stream[i];

	// forced_goal is now complete
	forced_goal = false;

	//printf("Goal pos: %1.2f, %1.2f\n", curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	update_errors();
}

void gestalt_force_goal(const Gestalt_goal_t* goal)
{
	forced_goal = true;
	curr_goal = *goal;
	update_errors();
}

// Returns the current action
Gestalt_action_t gestalt_get_current_action()
{
	return curr_goal.curr_action_goal;
}

// Returns the current status struct with all information for FSM and connectivity
Gestalt_status_t* gestalt_get_current_status() 
{
	return &curr_status;
}

// Returns the absolute position of the localization reference
Gestalt_vector2_t gestalt_get_lcl_ref_pos()
{
	Gestalt_vector2_t pos;
	pos.x = 0.f; pos.y = 0.f;
	return pos;
}

// Initialize timer
void gestalt_timer_init(uint8_t timer_number)
{
	if(timer_number == 2) {
		NRF_TIMER2->BITMODE |= 0x3;
		NRF_TIMER2->PRESCALER |= 0x4;
	}
	else if(timer_number == 3) {
		NRF_TIMER3->BITMODE |= 0x3;
		NRF_TIMER3->PRESCALER |= 0x4;
	}
	
}

// Reset the timer back to 0
void gestalt_timer_reset(uint8_t timer_number)
{
	if(timer_number == 2) {
		NRF_TIMER2->TASKS_CLEAR |= 0x1;
  		NRF_TIMER2->TASKS_START |= 0x1;
	}
	else if(timer_number == 3) {
		NRF_TIMER3->TASKS_CLEAR |= 0x1;
  		NRF_TIMER3->TASKS_START |= 0x1;
	}
}

// Get the current time passed since the last gestalt_timer_start
// Returns the time in microseconds
uint32_t gestalt_timer_read(uint8_t timer_number)
{
	if(timer_number == 2) {
		NRF_TIMER2->TASKS_CAPTURE[1] = 1;
		return (uint32_t)NRF_TIMER2->CC[1];
	}
	else if(timer_number == 3)
	{
		NRF_TIMER3->TASKS_CAPTURE[1] = 1;
		return (uint32_t)NRF_TIMER3->CC[1];
	}
	return 0;
	
}

// Fill the BLE buffer with all info according to the
// BLE broadcast packet definition
void gestalt_prep_ble_buffer(uint8_t* buffer)
{
	buffer[0] = _this_id;
	int32_t x = (int32_t)(curr_status.curr_pos.x * 10000.f);
	int32_t y = (int32_t)(curr_status.curr_pos.y * 10000.f);
	int32_t t = (int32_t)(curr_status.curr_theta * 10000.f);

	// x-pos
	buffer[1] = (uint8_t)(x >> 24);
	buffer[2] = (uint8_t)(x >> 16);
	buffer[3] = (uint8_t)(x >> 8);
	buffer[4] = (uint8_t)(x);

	// y-pos
	buffer[5] = (uint8_t)(y >> 24);
	buffer[6] = (uint8_t)(y >> 16);
	buffer[7] = (uint8_t)(y >> 8);
	buffer[8] = (uint8_t)(y);

	// theta
	buffer[9] = (uint8_t)(t >> 24);
	buffer[10] = (uint8_t)(t >> 16);
	buffer[11] = (uint8_t)(t >> 8);
	buffer[12] = (uint8_t)(t);

	// used to send localization pole positions
	// but this has been removed from the packet definition

	// path stream progress
	buffer[13] = curr_status.ps_progress;
	
}

// Parse the BLE buffer and populate corresponding records of other bot status
// Adheres to the BLE broadcast packet definition
void gestalt_parse_ble_buffer(uint8_t* buffer, uint8_t len) 
{
	float tmp_f;
	int32_t tmp_i;
	uint8_t o_id = buffer[7];
	bot_status_list[o_id-1].bot_id = o_id;

	// x-pos
	tmp_i = ( (int32_t)buffer[8] << 24 | (int32_t)buffer[9] << 16 | (int32_t)buffer[10] << 8 | (int32_t)buffer[11] );
	tmp_f = (float)tmp_i;
	tmp_f = tmp_f / 10000.f;
	bot_status_list[o_id-1].x = tmp_f;

	// y-pos
	tmp_i = ( (int32_t)buffer[12] << 24 | (int32_t)buffer[13] << 16 | (int32_t)buffer[14] << 8 | (int32_t)buffer[15] );
	tmp_f = ((float)tmp_i) / 10000.f;
	bot_status_list[o_id-1].y = tmp_f;

	// theta
	tmp_i = ( (int32_t)buffer[16] << 24 | (int32_t)buffer[17] << 16 | (int32_t)buffer[18] << 8 | (int32_t)buffer[19] );
	tmp_f = ((float)tmp_i) / 10000.f;
	bot_status_list[o_id-1].theta = tmp_f;

	// path stream progress
	bot_status_list[o_id-1].ps_progress = buffer[20];

	// mark the status list entry as valid
	bot_status_list[o_id-1].valid = 1;

	bot_status_list[o_id-1].last_sync_time = gestalt_timer_read(COMM_TIMER);

	//printf("%d %1.2f %1.2f\n", bot_status_list[o_id].bot_id, bot_status_list[o_id].x, bot_status_list[o_id].y);
	//	bot_status_list[o_id].theta);
}

// Get pointer to the status of other bots
Gestalt_bot_status_t* gestalt_get_status_list(void)
{
	return bot_status_list;
}