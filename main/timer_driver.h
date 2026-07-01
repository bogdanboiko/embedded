#pragma once
#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include <stdio.h>
#include <signal.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool is_timer_finished();

void clear_timer_finished_flag();

void start_timer_for_duration(uint32_t duration_ms);

void init_timer();

#endif