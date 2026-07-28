#pragma once
#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "display_commands.h"

typedef struct {
    uint32_t x;
    uint32_t y;
} canvas_point_t;

#define DISPLAY_ADDRESS 0x3C
#define DISPLAY_COL_NUMBER 128U
#define DISPLAY_ROW_NUMBER 64U
#define DISPLAY_PAGES_NUMBER 8U
#define DISPLAY_FRAME_BUFFER_SIZE (DISPLAY_COL_NUMBER * DISPLAY_PAGES_NUMBER)

void config_device_protocol(gpio_num_t sda_pin, gpio_num_t clk_pin);

void config_display();

bool troubleshoot_display_connection_success();

void clear_display_buffer();

void draw_line(canvas_point_t start, canvas_point_t end);

void draw_rect(canvas_point_t tl, canvas_point_t rb);

// void draw_curve();

#endif