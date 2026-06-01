#pragma once

#ifndef BUZZER_H
#define BUZZER_H

#include<stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define BUZZER_SOUND_LIBRARY_SIZE 7

typedef struct {
    char* name;
    float tone;
} buzzer_note_t;

typedef struct {
    buzzer_note_t note;
    uint16_t duty;
    uint32_t duratuion;
} buzzer_sound_t;

extern buzzer_note_t buzzer_sound_library[];

void buzzer_pwm_init(int gpio);

void buzzer_set_volume(float volume);

void buzzer_set_pitch(int pitchHz);

void buzzer_set_duty(int duty);

#endif