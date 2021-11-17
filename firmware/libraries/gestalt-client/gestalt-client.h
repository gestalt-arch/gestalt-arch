
#include "kobukiSensorTypes.h"

#define MAX_SOLUTION_LENGTH 64
#define MAX_BOTS 4

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
    Gestalt_action_t curr_action_goal;
} Gestalt_goal_t;

typedef struct {
    KobukiSensors_t* kobuki_sensors;
    YdLidarData_t* lidar_data;

} Gestalt_sensor_data_t;

typedef struct {
    float x_pos_stream[MAX_SOLUTION_LENGTH];
    float y_pos_stream[MAX_SOLUTION_LENGTH];
    int action_stream[MAX_SOLUTION_LENGTH];
    int exclusion_stream[MAX_SOLUTION_LENGTH];
    unsigned int path_length;
    int bot_id;
} Gestalt_path_stream_t;

typedef struct {
    Gestalt_path_stream_t* path_stream_vector[MAX_BOTS];
    unsigned int num_path_streams;
} Gestalt_path_stream_sol_t;

// Upload the pathstream solution
void gestalt_update_pathstream_sol(Gestalt_path_stream_sol_t* path);

// Receive sensor data
void gestalt_update_sensor_data(const Gestalt_sensor_data_t* sensor_data);

// Returns the current goal position and action
Gestalt_goal_t gestalt_get_current_goal();

Gestalt_status_t gestalt_get_current_status();

// Retur