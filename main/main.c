#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"
#include "ledc_controller.h"
#include "state_machine.h"

static const char* TAG = "example";

#define MOTOR_GPIO 36
#define BUTTON_GPIO 11  

void app_main(void) {
    button_init(BUTTON_GPIO);
    pwm_init(MOTOR_GPIO);
    
    init_state();
    start_polling_process();
}
