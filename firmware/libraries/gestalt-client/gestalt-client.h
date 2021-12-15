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
    uint8_t bot_id;
    float x;
    float y;
    float theta;
    int8_t ps_progress;
    bool valid;
} Gestalt_bot_status_t;

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


// Get the distance between two coordinates
float gestalt_get_2d_dist(float x1, float y1, float x2, float y2);

// Returns the provided vector translated by a distance in direction theta
void gestalt_transform_vector(Gestalt_vector2_t* vec, float dist, float theta);

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

// Force gestalt to provide pathing for a user-specified goal
void gestalt_force_goal(const Gestalt_goal_t* goal);

// Returns the current action
Gestalt_action_t gestalt_get_current_action();

// Returns the current status struct with all information for FSM and connectivity
Gestalt_status_t* gestalt_get_current_status();

// Returns the absolute position of the localization reference
Gestalt_vector2_t gestalt_get_lcl_ref_pos();

// Initialize timer
void gestalt_timer_init();

// Reset the timer back to 0
void gestalt_timer_reset();

// Get the current time passed since the last gestalt_timer_start
// Returns the time in microseconds
int32_t gestalt_timer_read();

// Fill the BLE buffer with all info according to the
// BLE broadcast packet definition
void gestalt_prep_ble_buffer(uint8_t* buffer);

// Parse the BLE buffer and populate corresponding records of other bot status
// Adheres to the BLE broadcast packet definition
void gestalt_parse_ble_buffer(uint8_t* buffer, uint8_t len);

// Get pointer to the status of other bots
Gestalt_bot_status_t* gestalt_get_status_list(void);

