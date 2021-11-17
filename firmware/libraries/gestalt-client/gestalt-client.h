
#include "kobukiSensorTypes.h"

#define MAX_PATH 128

typedef enum {
    GESTALT_MOVE,
    GESTALT_GRAB,
    GESTALT_DROP
} Gestalt_action_t;

typedef struct {
    float curr_x_pos;
    float curr_y_pos;
    float curr_rotation;
} Gestalt_status_t;

typedef struct {
    float curr_x_goal;
    float curr_y_goal;
} Gestalt_goal_t;

typedef struct {
    KobukiSensors_t* kobuki_sensors;
    YdLidarData_t* lidar_data;

} Gestalt_sensor_data_t;

void gestalt_update_pathstream

// Receive sensor data
void gestalt_update_sensor_data(const Gestalt_sensor_data_t* sensor_data);

// Return action
Gestalt_goal_t gestalt_get_current_goal();

Gestalt_status_t gestalt_get_current_status();

// Retur