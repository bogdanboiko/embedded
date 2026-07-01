#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_driver.h"
#include "traffic_config.h"
#include "traffic_light_driver.h"
#include "traffic_state_machine.h"

static const char* TAG = "traffic_light";

static traffic_light_t SN = {
    .red_led_gpio = 4, .yellow_led_gpio = 5, .green_led_gpio = 6};

static traffic_light_t WE = {
    .red_led_gpio = 7, .yellow_led_gpio = 15, .green_led_gpio = 16};

void update_hardware() {
    traffic_phase_config_t config = get_state_config(get_current_state());
    set_traffic_state(&SN, config.lights_config >> NS_LIGHTS_BASE);
    set_traffic_state(&WE, config.lights_config >> EW_LIGHTS_BASE);

    start_timer_for_duration(config.duration_ms);
}

void app_main(void) {
    config_hadrware_for_traffic_light(&SN);
    config_hadrware_for_traffic_light(&WE);
    init_timer();
    update_hardware();

    while (1) {
        if (is_timer_finished()) {
            proceed_to_next_state();
            clear_timer_finished_flag();
            update_hardware();
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
