/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rmt_driver.h"
#include "measurement_constants.h"

#define RMT_FREQ (300 * HZ_TO_KILO)
#define OUTPUT_PIN 4

static const char* TAG = "RMT_driver";

/*
    To calculate single tick time you need rmt_raw_init frequency value 1/RMT_FREQ
*/

void send_items_demo() {
    rmt_item_t items[2] = {
        {.dur0 = 5000, .lvl0 = 1, .dur1 = 5000, .lvl1 = 0},
        {.dur0 = 5000, .lvl0 = 1, .dur1 = 5000, .lvl1 = 0},
    };
    rmt_raw_send_items(items, 2);
}

void send_pulses_demo() {
    uint32_t items[4] = {20, 50, 100, 20};
    rmt_raw_send_pulses(items, 4, 1);
}

void send_square_demo() { 
    rmt_raw_square(50 * HZ_TO_KILO, 50, 100);
}

void app_main(void) {
    rmt_raw_init(OUTPUT_PIN, RMT_FREQ);
    while (1) {
        // send_items_demo();
        // send_pulses_demo();
        // send_square_demo();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
