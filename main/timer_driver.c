#include "timer_driver.h"

volatile bool timer_finished = false;

static esp_timer_handle_t fsm_timer;

void IRAM_ATTR timer_isr(void *arg)
{
    timer_finished = true;
}

bool is_timer_finished() {
    return timer_finished;
}

void clear_timer_finished_flag() {
    timer_finished = false;
}

void init_timer() {
     esp_timer_create_args_t args = {
        .callback = &timer_isr,
        .name = "fsm_timer"
    };

    esp_timer_create(&args, &fsm_timer);
}

void start_timer_for_duration(uint32_t duration_ms) {
    esp_timer_stop(fsm_timer);

    esp_timer_start_once(
        fsm_timer,
        duration_ms * 1000
    );
}