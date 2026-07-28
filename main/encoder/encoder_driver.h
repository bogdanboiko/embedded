#pragma once
#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

void config_encoder(gpio_num_t cn_a_gpio, gpio_num_t cn_b_gpio, gpio_num_t sw);

int get_current_count();

int get_current_angle();

#endif