
#include "kobukiSensorTypes.h"
#include <stdint.h>

#define MAX_SOLUTION_LENGTH 64
#define MAX_BOTS 3

typedef enum {
    GESTALT_MOVE,
    GESTALT_GRAB,
    GESTALT_DROP,
    GESTALT_STOP
} Gestalt_action_t;

typedef struct {
    float pos_error;
    float theta_error;
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
    int32_t action_stream[MAX_SOLUTION_LENGTH];
    int32_t exclusion_stream[MAX_SOLUTION_LENGTH];
    uint32_t path_length;
    int32_t bot_id;
} Gestalt_path_stream_t;

typedef struct {
    Gestalt_path_stream_t path_stream_vector[MAX_BOTS];
    uint8_t num_path_streams;
} Gestalt_path_stream_sol_t;

// After completing the serial read, deserialize the buffer into a Gestalt_path_stream_sol_t
void gestalt_deserialize_solution(uint8_t* solution_buffer, uint16_t solution_num_bytes);

// Receive sensor data
void gestalt_update_sensor_data(const Gestalt_sensor_data_t* sensor_data);

// Inform gestalt client that the active goal is complete
void gestalt_send_goal_complete();

// Returns the current goal position and action
Gestalt_goal_t gestalt_get_current_goal();

Gestalt_status_t gestalt_get_current_status();

// Retur
