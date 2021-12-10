#include "ydlidar_x2.h"
#include "kobukiSensorTypes.h"
#include <stdint.h>
#include <math.h>

#define MAX_SOLUTION_LENGTH 8
#define MAX_BOTS 3

typedef enum {
    GESTALT_MOVE,
    GESTALT_GRAB,
    GESTALT_DROP,
    GESTALT_STOP
} Gestalt_action_t;

typedef struct {
    float x;
    float y;
} Gestalt_vector2_t;

typedef struct {
    // For use in reference FSM
    float pos_error;      // Euclidean distance between current position and goal position
    float theta_error;    // Theta between current orientation vector and goal orientation vector
    // For use in connectivity
    uint8_t bot_id;       // Unique Bot ID
    Gestalt_vector2_t curr_pos; // Current position as (x, y) localized in shared global space
    float curr_theta;     // Current orientation (in degrees) localized in shared global space
    int8_t ps_progress;  // Path stream progress, indicates which waypoint on the path stream
                         // last accomplished. Immediately after initialization, this will be -1
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
    uint8_t path_length;
    uint8_t bot_id;
    float x_pos_stream[MAX_SOLUTION_LENGTH];
    float y_pos_stream[MAX_SOLUTION_LENGTH];
    int32_t action_stream[MAX_SOLUTION_LENGTH];
    int32_t exclusion_stream[MAX_SOLUTION_LENGTH];
} Gestalt_path_stream_t;

typedef struct {
    uint8_t num_path_streams;
    Gestalt_path_stream_t path_stream_vector[MAX_BOTS];
} Gestalt_path_stream_sol_t;

// After completing the serial read, deserialize the buffer into a Gestalt_path_stream_sol_t
void gestalt_deserialize_solution(uint8_t* solution_buffer, uint16_t solution_num_bytes);

// Initialize the gestalt client
// Must be called AFTER providing the deserialized solution via gestalt_deserialize_solution
// Must be called BEFORE calling any other gestalt client functions
// 
// Provide the bot id
void gestalt_init(uint8_t bot_id, KobukiSensors_t* kobuki_sensors);

// Update the sensor data and all internal state space representations
void gestalt_update_sensor_data(KobukiSensors_t* kobuki_sensors);

// Inform gestalt client that the active goal is complete
void gestalt_send_goal_complete();

// Returns the current action
Gestalt_action_t gestalt_get_current_action();

// Returns the current status struct with all information for FSM and connectivity
Gestalt_status_t* gestalt_get_current_status();

// Returns the absolute position of the localization reference
Gestalt_vector2_t gestalt_get_lcl_ref_pos();
