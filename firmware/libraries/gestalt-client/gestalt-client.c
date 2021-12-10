#include "gestalt-client.h"


// The complete path stream solution
static Gestalt_path_stream_sol_t ps_solution;

// The path stream for this bot to follow
// determined after
static Gestalt_path_stream_t target_ps;

static Gestalt_status_t curr_status;



static void deserialize_path_stream(uint8_t* path_stream, uint32_t path_length, uint8_t ps_idx) {
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


// Initialize the gestalt client
// Must be called AFTER providing the deserialized solution via gestalt_deserialize_solution
// Must be called BEFORE calling any other gestalt client functions
// 
// Provide the bot id
void gestalt_init(uint8_t bot_id) {
	curr_status.pos_error = 0.f;
	curr_status.theta_error = 0.f;
	curr_status.bot_id = bot_id;
	curr_status.curr_pos.x = target_ps.x_pos_stream[0];
	curr_status.curr_pos.y = target_ps.y_pos_stream[0];
	curr_status.curr_theta = 0.f;
	curr_status.ps_progress = -1;
}

// Update the sensor data and all internal state space representations
void gestalt_update_sensor_data(const Gestalt_sensor_data_t* sensor_data)
{

}

// Inform gestalt client that the active goal is complete
void gestalt_send_goal_complete()
{

}

// Returns the current action
Gestalt_action_t gestalt_get_current_action()
{
	return GESTALT_MOVE;
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