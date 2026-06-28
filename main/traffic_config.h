#pragma once
#ifndef TRAFFIC_CONFIG_H
#define TRAFFIC_CONFIG_H

#include <stdio.h>
#include "traffic_states.h"
#include "traffic_config.h"

/*
 lights configured with 8 bits(4 bits for ns, and 4 bits for ew)
 1 bit is for Red
 2 bit is for Yellow
 3 bit is for Green
 4 bit is for blinking state
*/
#define NS_LIGHTS_BASE 4
#define EW_LIGHTS_BASE 0

#define RED (1u << 3)
#define YELLOW (1u << 2)
#define GREEN (1u << 1)
#define BLINKING (1u << 0)

typedef struct {
    uint32_t duration_ms;
    uint8_t lights_config;
} traffic_phase_config_t;

traffic_phase_config_t get_state_config(traffic_state_t state);
#endif