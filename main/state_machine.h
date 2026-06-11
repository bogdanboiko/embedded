#pragma once

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include<stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ledc_controller.h"
#include "button.h"

void init_state();

void start_polling_process();

#endif