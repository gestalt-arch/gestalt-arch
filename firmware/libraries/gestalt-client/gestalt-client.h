
#include "kobukiSensorTypes.h"



typedef struct {
    KobukiSensors_t* kobuki_sensors;
    YdLidarData_t* lidar_data;

} Gestalt_sensor_data_t;

// Receive sensor data 

void gestalt_update_sensor_data(Gestalt_sensor_data_t* sensor_data);


// Return action

// Retur