#pragma once

#ifndef BUZZER_H
#define BUZZER_H

#include<stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void buzzer_pwm_init(int gpio);

void buzzer_set_volume(float volume);

void buzzer_set_pitch(int pitchHz);

#endif