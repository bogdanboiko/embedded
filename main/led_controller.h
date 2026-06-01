#pragma once

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include<stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void config_led_pwm(int gpio_pin);

void set_brightness_level(float level);

#endif