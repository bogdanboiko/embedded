#pragma once

#ifndef BUTTON_H
#define BUTTON_H
#include<stdio.h>
#include<stdbool.h>

void button_init(int gpio_num);
bool button_get_state(void);
__uint32_t get_button_press_count(void);

#endif