#include "traffic_state_machine.h"

traffic_state_t current_state;

void proceed_to_next_state() {
    current_state = (current_state + 1) % TRAFFIC_STATE_AMOUNT;
}

traffic_state_t get_current_state() {
    return current_state;
}