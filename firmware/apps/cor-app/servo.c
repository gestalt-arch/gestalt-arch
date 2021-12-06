// Select a pin where we want to connect the servo's pin for pwm
#define  SERVO_PIN   4

#define TIME_PERIOD  3030 // 3.03 milliseconds

// set the duty in milliseconds time period to control its position
// Define open and close duty cycles for servos

#define OPEN_CYCLE  0.5

#define CLOSE_CYLCE 2.5

void init(void) {
  // Create a pwm instance by passing the name of the handle and the timer to be used for pwm generation
  // here 1 means timer 1
  APP_PWM_INSTANCE(PWM1, 1);
  // a variable to hold err_code value
  ret_code_t err_code;

//  create a pwm config struct and pass it the default configurations along with the pin number for pwm and period in microseconds
// here we have configured one channel, we can configure max up to two channels per pwm instance in this library, if we use two channels then we need to pass two pins
  app_pwm_config_t pwm_cfg = APP_PWM_DEFAULT_CONFIG_1CH(TIME_PERIOD, SERVO_PIN);

// change the pwm polarity by setting this value so that logic high value is given as active signal and duty is set for the logic high signal
// we can change it to give the active low signal as well for manipulating the logic low duty
  pwm_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

// Initialize the pwm and pass it the intance, configurations and handler, we can also write NULL if we don't want to use the handler 
  err_code = app_pwm_init(&PWM1, &pwm_cfg, NULL);
  APP_ERROR_CHECK(err_code);// check if any error occurred during initialization
// enable the pwm signal so that the pwm is started on the specified pin
  app_pwm_enable(&PWM1);
}



void release(void) {
  while(app_pwm_channel_duty_set(&PWM1, 0, OPEN_CYCLE) == NRF_ERROR_BUSY);
}

void capture(void) {
  while(app_pwm_channel_duty_set(&PWM1, 0, CLOSE_CYLCE) == NRF_ERROR_BUSY);
}


/** @} */
