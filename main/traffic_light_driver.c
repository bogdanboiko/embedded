#include "traffic_light_driver.h"

#define BLINK_DURATION_INTERVAL 1000

TaskHandle_t blink_task_handler;

volatile long blink_gpio_map = 0;

void blink_task(void* args) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50));
        if (blink_gpio_map != 0) {
            for (int n = 0; n < GPIO_NUM_MAX; n++) {
                if (blink_gpio_map & (1u << n)) {
                    gpio_set_level(n, 1);
                }
            }

            vTaskDelay(pdMS_TO_TICKS(BLINK_DURATION_INTERVAL));

            for (int n = 0; n < GPIO_NUM_MAX; n++) {
                if (blink_gpio_map & (1u << n)) {
                    gpio_set_level(n, 0);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(BLINK_DURATION_INTERVAL));
        }
    }
}

void config_led_gpio(uint8_t gpio) {
    gpio_reset_pin(gpio);

    gpio_config_t config = {.pin_bit_mask = 1u << gpio,
                            .intr_type = GPIO_INTR_DISABLE,
                            .mode = GPIO_MODE_OUTPUT,
                            .pull_down_en = false,
                            .pull_up_en = false};

    gpio_config(&config);
}

void config_hadrware_for_traffic_light(traffic_light_t* config) {
    config_led_gpio(config->green_led_gpio);
    config_led_gpio(config->yellow_led_gpio);
    config_led_gpio(config->red_led_gpio);
}

void set_traffic_state(traffic_light_t* config, uint8_t lights_config) {
    bool isBlinking = lights_config & BLINKING;
    bool isRedOn = lights_config & RED;
     bool isYellowOn = lights_config & YELLOW;
      bool isGreenOn = lights_config & GREEN;

    if (isBlinking) {
        if (blink_task_handler == NULL) {
            xTaskCreate(blink_task, "BlinkTask", 1000, NULL, 4,
                        &blink_task_handler);
        }

        blink_gpio_map |= (isRedOn << config->red_led_gpio) |
                          (isGreenOn << config->green_led_gpio) |
                          (isYellowOn << config->yellow_led_gpio);
    } else {
        blink_gpio_map &= ~((1u << config->red_led_gpio) |
                          (1u << config->green_led_gpio) |
                          (1u << config->yellow_led_gpio));
        gpio_set_level(config->green_led_gpio, isGreenOn);
        gpio_set_level(config->yellow_led_gpio, isYellowOn);
        gpio_set_level(config->red_led_gpio, isRedOn);
    }
}