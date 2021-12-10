#include "gestalt-client.h"


// The complete path stream solution
static Gestalt_path_stream_sol_t ps_solution;

// The path stream for this bot to follow
// determined after
static Gestalt_path_stream_t target_ps;

// Contains all info regarding the current status of the robot
static Gestalt_status_t curr_status;

// Contains the goal position and action of the bot
static Gestalt_goal_t curr_goal;

// Keep history of encoder values
static uint16_t prev_encoder_left;
static uint16_t prev_encoder_right;

#define STRAIGHT_DIST_THRESHOLD 0.05

inline static float get_2d_dist(float x1, float y1, float x2, float y2)
{
	return sqrtf(powf(x2-x1, 2) + powf(y2-y1, 2));
}

inline static float get_2d_theta(float x1, float y1, float x2, float y2)
{
	return atan2f(y2 - y1, x2 - x1) * (180.f / M_PI);
}

inline static float get_encoder_dist(uint16_t curr_encoder, uint16_t prev_encoder) 
{
	const float CONVERSION = 0.0006108f;
	float dist;
	if(curr_encoder < prev_encoder)
		dist = (0xFFFF - prev_encoder) + curr_encoder;
	else
		dist = curr_encoder - prev_encoder;
	return dist * CONVERSION;
}

inline static void update_curr_pos(float dist, float theta) 
{
	float theta_rad = theta * (M_PI / 180.f);
	// tokyo drift
	curr_status.curr_pos.x += dist * cosf(theta_rad);
	curr_status.curr_pos.y += dist * sinf(theta_rad); 
}

inline static void update_errors()
{
	float dist = get_2d_dist(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.pos_error = dist;
	float theta = get_2d_theta(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.theta_error = theta;
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
	for (int i = 0; i < ps_solution.num_path_streams; i++) {
		ps_solution.path_stream_vector[i].bot_id = i + 1;
		ps_solution.path_stream_vector[i].path_length = 4;
		
		ps_solution.path_stream_vector[i].x_pos_stream[0] = 0.0f;
		ps_solution.path_stream_vector[i].y_pos_stream[0] = 0.0f;
		ps_solution.path_stream_vector[i].action_stream[0] = GESTALT_MOVE;

		ps_solution.path_stream_vector[i].x_pos_stream[1] = 0.0f;
		ps_solution.path_stream_vector[i].y_pos_stream[1] = 0.5f;
		ps_solution.path_stream_vector[i].action_stream[1] = GESTALT_MOVE;

		ps_solution.path_stream_vector[i].x_pos_stream[2] = 0.5f;
		ps_solution.path_stream_vector[i].y_pos_stream[2] = 0.5f;
		ps_solution.path_stream_vector[i].action_stream[2] = GESTALT_MOVE;

		ps_solution.path_stream_vector[i].x_pos_stream[3] = 0.0f;
		ps_solution.path_stream_vector[i].y_pos_stream[3] = 0.0f;
		ps_solution.path_stream_vector[i].action_stream[3] = GESTALT_MOVE;
	}

}


// Initialize the gestalt client
// Must be called AFTER providing the deserialized solution via gestalt_deserialize_solution
// Must be called BEFORE calling any other gestalt client functions
// 
// Provide the bot id
void gestalt_init(uint8_t bot_id, KobukiSensors_t* kobuki_sensors) 
{
	gestalt_init_test_path(); // debug only

	// find path assigned to this bot
	// should probably search in the future
	target_ps = ps_solution.path_stream_vector[bot_id-1];

	// initialize current position/theta, ID, and progress
	curr_status.curr_pos.x = target_ps.x_pos_stream[0];
	curr_status.curr_pos.y = target_ps.y_pos_stream[0];
	curr_status.curr_theta = 0.f;
	curr_status.bot_id = bot_id;
	curr_status.ps_progress = 0;

	// init goals
	curr_goal.curr_x_goal = target_ps.x_pos_stream[1];
	curr_goal.curr_y_goal = target_ps.y_pos_stream[1];
	curr_goal.curr_action_goal = target_ps.action_stream[1];

	// save current encoder values
	prev_encoder_left = kobuki_sensors->leftWheelEncoder;
	prev_encoder_right = kobuki_sensors->rightWheelEncoder;

	update_errors();
}

// Update the sensor data and all internal state space representations
void gestalt_update_sensor_data(KobukiSensors_t* kobuki_sensors)
{
	float dist_left = get_encoder_dist(kobuki_sensors->leftWheelEncoder, prev_encoder_left);
	float dist_right = get_encoder_dist(kobuki_sensors->rightWheelEncoder, prev_encoder_right);

	float dist_diff, dist;
	dist_diff = fabs(dist_left - dist_right);

	// only increment distance if the wheels were traveling approximately straight
	dist = (dist_diff <= STRAIGHT_DIST_THRESHOLD) ? (dist_left + dist_right) / 2 : 0.f;

	// debug
	printf("Dist traveled: %1.4f\n", dist); 

	// save current encoder values
	prev_encoder_left = kobuki_sensors->leftWheelEncoder;
	prev_encoder_right = kobuki_sensors->rightWheelEncoder;

	printf("Angle: %d Angle rate: %d\n", (int32_t)kobuki_sensors->angle, (int32_t)kobuki_sensors->angleRate);

	// attempt to integrate theta
	float new_theta = curr_status.curr_theta + (kobuki_sensors->angleRate * 0.001f); 

	update_curr_pos(dist, new_theta);
	// update current theta
	curr_status.curr_theta = new_theta;

	update_errors();
}

// Inform gestalt client that the active goal is complete
void gestalt_send_goal_complete()
{
	// increment ps progress
	if(curr_status.ps_progress != target_ps.path_length)
		curr_status.ps_progress += 1;
	// update goals
	uint8_t ps_prog = curr_status.ps_progress;
	curr_goal.curr_x_goal = target_ps.x_pos_stream[ps_prog + 1];
	curr_goal.curr_y_goal = target_ps.y_pos_stream[ps_prog + 1];
	curr_goal.curr_action_goal = target_ps.action_stream[ps_prog + 1];

	float dist = get_2d_dist(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.pos_error = dist;
	float theta = get_2d_theta(curr_status.curr_pos.x, curr_status.curr_pos.y,
		curr_goal.curr_x_goal, curr_goal.curr_y_goal);
	curr_status.theta_error = theta;
}

// Returns the current action
Gestalt_action_t gestalt_get_current_action()
{
	return curr_goal.curr_action_goal;
	//return GESTALT_MOVE;
}

// Returns the current status struct with all information for FSM and connectivity
Gestalt_status_t gestalt_get_current_status() 
{
	return curr_status;
}

// Returns the absolute position of the localization reference
Gestalt_vector2_t gestalt_get_lcl_ref_pos()
{
	Gestalt_vector2_t pos;
	pos.x = 0.f; pos.y = 0.f;
	return pos;
}