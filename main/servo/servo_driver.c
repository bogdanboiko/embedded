#include "servo_driver.h"

#define SERVO_PROTOCOL_HZ 50
#define SERVO_PROTOCOL_PERIOD_US 20000
#define SERVO_POSITION_MINUS_90_US 400
#define SERVO_POSITION_0_US 1250
#define SERVO_POSITION_90_US 2100
#define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY ((1 << LEDC_DUTY_RES) - 1)


void config_servo_control(gpio_num_t gpio) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_SPEED_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = SERVO_PROTOCOL_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_SPEED_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .gpio_num = gpio,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

int angle_to_duty(int angle) {
    int duty_step = SERVO_PROTOCOL_PERIOD_US / (LEDC_MAX_DUTY + 1);
    int min_duty_for_angle = SERVO_POSITION_MINUS_90_US / duty_step;
    int max_duty_for_angle = SERVO_POSITION_90_US / duty_step;

    if (angle < -90) {
        return min_duty_for_angle;
    } else if (angle > 90) {
        return max_duty_for_angle;
    } else {
        int duty = min_duty_for_angle + ((angle + 90) * (max_duty_for_angle - min_duty_for_angle) / 180);
         ESP_LOGI("servo", "Duty: %d", duty * duty_step);
         return duty;
    }
}

void set_servo_angle(int angle) {
    ledc_set_duty(LEDC_SPEED_MODE, LEDC_CHANNEL, angle_to_duty(angle));
    ledc_update_duty(LEDC_SPEED_MODE, LEDC_CHANNEL);
}