#include "cor-app.h"
#include <math.h>

#define DRIVE_SPEED 100
#define TURN_SPEED 100

// initialize state and state variables
KobukiState_t state = STOP;
KobukiSensors_t sensors = { 0 };
uint8_t timer = 0;
Gestalt_status_t status;
float cur_pos_error;
float cur_theta_error;
Gestalt_action_t action;

void corapp_run()
{
	// read sensors from robot
	kobukiSensorPoll(&sensors);
	gestalt_update_sensor_data(&sensors);

	status = gestalt_get_current_status();
	cur_pos_error = status.pos_error;
    cur_theta_error = status.theta_error;

	char disp_buffer[32];
	sprintf(disp_buffer, "pos: %1.2f, %1.2f", 
		status.curr_pos.x, status.curr_pos.y);
	display_write(disp_buffer, DISPLAY_LINE_1);

    action = gestalt_get_current_action();

	// test current state
	switch (state) {
		case STOP:
			//	STOP to ALIGN_CW
			if (action == GESTALT_MOVE && cur_theta_error < 0) {
				state = ALIGN_CW;
			}
			// STOP to ALIGN_CCW
			else if (action == GESTALT_MOVE && cur_theta_error > 0) {
				state = ALIGN_CCW;
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
			if (cur_theta_error <= -0.5) {
				state = ALIGN_CW;
			}
			//	DRIVE to ALIGN_CCW
			else if (cur_theta_error >= 0.5) {
				state = ALIGN_CCW;
			}
			//	DRIVE
			else if (cur_pos_error <= 0.1) {
				state = STOP;
				gestalt_send_goal_complete();
				kobukiDriveDirect(0, 0);
			}
			else {
				display_write("DRIVE", DISPLAY_LINE_0);
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
				kobukiDriveDirect(-DRIVE_SPEED, DRIVE_SPEED);
			}
			break;
		case ALIGN_CCW:
			// ALIGN_CCW to DRIVE
			if (fabs(cur_theta_error) <= 0.5) {
				state = DRIVE;
			}
			// ALIGN_CCW to ALIGN_CCW
			else {
				display_write("ALIGN CCW", DISPLAY_LINE_0);
				kobukiDriveDirect(DRIVE_SPEED, -DRIVE_SPEED);
			}
			break;
	}
	// continue for 10 ms before checking state again
}