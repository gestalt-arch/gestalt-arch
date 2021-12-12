// cor-app
#include "cor-app.h"

#define GESTALT_BOT_ID 1

int main(void) {

	// initialize Kobuki library
	kobukiInit();


	// initialize cor-app
	// handles all peripheral init
	corapp_init();

	KobukiSensors_t sensors;
	kobukiSensorPoll(&sensors);

	// Initialize gestalt-client
	gestalt_init(GESTALT_BOT_ID, &sensors);

	while (1) {
		corapp_run();
		nrf_delay_ms(1);
	}
}

