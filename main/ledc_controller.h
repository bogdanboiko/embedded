#pragma once

#ifndef LEDC_CONTROLLER_H
#define LEDC_CONTROLLER_H

#include<stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pwm_init(int gpio);

void set_rotation_speed_percent(float volume);

#endif