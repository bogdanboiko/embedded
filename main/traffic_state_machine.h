#pragma once
#ifndef TRAFFIC_STATE_MACHINE_H
#define TRAFFIC_STATE_MACHINE_H

#include <stdio.h>

#include "traffic_states.h"
#include "traffic_config.h"

void init_state();

void proceed_to_next_state();

traffic_state_t get_current_state();

#endif