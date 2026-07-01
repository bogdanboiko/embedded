#pragma once
#ifndef TRAFFIC_LIGHT_DRIVER_H
#define TRAFFIC_LIGHT_DRIVER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "traffic_config.h"

typedef struct {
    uint8_t red_led_gpio;
    uint8_t yellow_led_gpio;
    uint8_t green_led_gpio;
} traffic_light_t;

void config_hadrware_for_traffic_light(traffic_light_t *config);

void set_traffic_state(traffic_light_t *config, uint8_t lights_config);

#endif