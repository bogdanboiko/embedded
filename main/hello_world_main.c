/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN0 4
#define PIN1 13
#define PIN2 14
#define PIN3 16
#define PIN4 17

static const char *TAG = "frequency_gen";
static short c0, c1, c2, c3, c4 = 0;




static void config_gpio(void) {
    ESP_LOGI(TAG, "Start config");
    gpio_reset_pin(PIN0);
    gpio_reset_pin(PIN1);
    gpio_reset_pin(PIN2);
    gpio_reset_pin(PIN3);
    gpio_reset_pin(PIN4);

    gpio_set_direction(PIN0, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(PIN1, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(PIN2, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(PIN3, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(PIN4, GPIO_MODE_INPUT_OUTPUT);

    gpio_set_level(PIN0, 0);
    gpio_set_level(PIN1, 0);
    gpio_set_level(PIN2, 0);
    gpio_set_level(PIN3, 0);
    gpio_set_level(PIN4, 0);
    ESP_LOGI(TAG, "Finished config pins!");
}

void app_main(void) {
    config_gpio();

    while (1) {
        c0++;
        c1++;
        c2++;
        c3++;
        c4++;

        if (c0 >= 10) {
            ESP_LOGI(TAG, "PIN0 before: %d", gpio_get_level(PIN0));
            int btn_state = gpio_get_level(PIN0);
            gpio_set_level(PIN0, !btn_state);
            ESP_LOGI(TAG, "PIN0 after: %d", gpio_get_level(PIN0));
            c0 = 0;
        }
        if (c1 >= 50) {
            gpio_set_level(PIN1, !gpio_get_level(PIN1));
            c1 = 0;
        }
        if (c2 >= 100) {
            gpio_set_level(PIN2, !gpio_get_level(PIN2));
            c2 = 0;
        }
        if (c3 >= 150) {
            gpio_set_level(PIN3, !gpio_get_level(PIN3));
            c3 = 0;
        }
        if (c4 >= 200) {
            gpio_set_level(PIN4, !gpio_get_level(PIN4));
            c4 = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
