#include "led_controller.h"
#include "esp_log.h"

static int selected_led_gpio = 35;

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT

#define LEDC_CLK_SRC LEDC_USE_RC_FAST_CLK  

void config_led_pwm(int gpio_pin) {
    selected_led_gpio = gpio_pin;
    ledc_timer_config_t timer_config = {
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .speed_mode = LEDC_MODE,
        .freq_hz = 100,
        .clk_cfg = LEDC_CLK_SRC
    };

    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .gpio_num = selected_led_gpio,
        .duty = 0,
        .hpoint = 0,
    };

    ledc_channel_config(&channel_config);
}

void set_brightness_level(float levelPercent) {
    int brightness_level = (1 << LEDC_DUTY_RES) * levelPercent;
            ESP_LOGI("Test", "set brightness of led %d", brightness_level);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness_level);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}