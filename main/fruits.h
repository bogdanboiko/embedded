#pragma once

#ifndef FRUITS_H
#define FRUITS_H

#include "driver/touch_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void fruits_init(bool enable_debug_monitoring);

bool fruits_is_touching(void);

uint16_t fruits_read_pot(void);

int fruits_active_index(void);

char *fruits_name(int i);

int fruits_count(void);

#endif