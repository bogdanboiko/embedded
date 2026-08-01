#pragma once
#ifndef SERVO_DRIVER_H
#define SERVO_DERIVER_H

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

void config_servo_control(gpio_num_t gpio);

void set_servo_angle(int angle);

#endif