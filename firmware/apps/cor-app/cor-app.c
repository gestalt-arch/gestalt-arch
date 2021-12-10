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

static int8_t turn_speed = TURN_SPEED;

void corapp_run()
{
	// read sensors from robot
	kobukiSensorPoll(&sensors);
	gestalt_update_sensor_data(&sensors);

	status = gestalt_get_current_status();
	cur_pos_error = status->pos_error;
    cur_theta_error = status->theta_error;

	char disp_buffer[32];

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
				sprintf(disp_buffer, "pos: %1.2f, %1.2f", 
					status->curr_pos.x, status->curr_pos.y);
				display_write(disp_buffer, DISPLAY_LINE_1);
				kobukiDriveDirect(DRIVE_SPEED, DRIVE_SPEED);
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
				sprintf(disp_buffer, "t: %1.2f, t_e %1.2f", status->curr_theta, cur_theta_error);
				display_write(disp_buffer, DISPLAY_LINE_1);
				kobukiDriveDirect(turn_speed, -turn_speed);
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
				sprintf(disp_buffer, "t: %1.2f, t_e %1.2f", status->curr_theta, cur_theta_error);
				display_write(disp_buffer, DISPLAY_LINE_1);
				kobukiDriveDirect(-turn_speed, turn_speed);
			}
			break;
	}
	// continue for 10 ms before checking state again
}