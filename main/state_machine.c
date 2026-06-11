#include "state_machine.h"

typedef enum { OFF, LOW, MEDIUM, FULL } motor_state;

typedef struct {
    bool is_button_pressed;
} motor_state_data;

motor_state current_state;
motor_state_data state_data;

void init_state() {
     current_state = OFF;
     state_data.is_button_pressed = false;
 }



void start_polling_process() {
    while (1) {
        bool is_button_pressed = button_get_state();
        
        if (!state_data.is_button_pressed) {
            state_data.is_button_pressed = is_button_pressed;
        }

        if (state_data.is_button_pressed && !is_button_pressed) {
            motor_state init_state = current_state;
            state_data.is_button_pressed = is_button_pressed;

            switch (current_state) {
                case OFF:
                    current_state = LOW;
                    set_rotation_speed_percent(0.7);
                    break;

                case LOW:
                    current_state = MEDIUM;
                    set_rotation_speed_percent(0.85);
                    break;

                case MEDIUM:
                    current_state = FULL;
                    set_rotation_speed_percent(1);
                    break;

                case FULL:
                    current_state = OFF;
                    set_rotation_speed_percent(0);
                    break;
                default:
                    current_state = OFF;
                    set_rotation_speed_percent(0);
                    break;    
            }

            ESP_LOGI("Test", "State before was %d and became: %d", init_state, current_state);
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}