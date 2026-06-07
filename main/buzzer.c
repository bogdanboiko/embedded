#include "buzzer.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT

#define LEDC_CLK_SRC LEDC_USE_RC_FAST_CLK  

void buzzer_pwm_init(int gpio) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = 100,
        .clk_cfg = LEDC_CLK_SRC,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_MODE,
                                          .channel = LEDC_CHANNEL,
                                          .timer_sel = LEDC_TIMER,
                                          .gpio_num = gpio,
                                          .duty = 0,
                                          .hpoint = 0};

    ledc_channel_config(&ledc_channel);
}

int calculate_duty_for_volume(ledc_timer_bit_t resolution, float volume) {
    return (1 << resolution) * volume;
}

void buzzer_set_volume(float volume) {
    int duty = calculate_duty_for_volume(LEDC_DUTY_RES, volume);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void buzzer_set_pitch(int pitchHz) {
    ledc_set_freq(LEDC_MODE, LEDC_CHANNEL, pitchHz);
}