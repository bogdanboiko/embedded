#include <stdio.h>

#include "button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char* TAG = "example";

#define BLINK_GPIO 11
#define BUTTON_GPIO 35

#define BLINK_PERIOD_MS 1000

static __uint8_t led_state = 0;

static TaskHandle_t blink_task_handle;
static TaskHandle_t button_task_handle;

static void configure_led(void) {
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_config_t io_config = {.pin_bit_mask = (1ULL << BLINK_GPIO),
                               .mode = GPIO_MODE_OUTPUT,
                               .pull_up_en = 0,
                               .pull_down_en = 0,
                               .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_config);
    gpio_set_level(BLINK_GPIO, led_state);
}

// static bool is_button_pressed(void) {
//     if (gpio_get_level(BUTTON_GPIO) != 0) {
//         return false;
//     }

//     vTaskDelay(pdMS_TO_TICKS(10));
//     if (gpio_get_level(BUTTON_GPIO) != 0) {
//         return false;
//     }

//     while (gpio_get_level(BUTTON_GPIO) == 0) {
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }

//     return true;
// }

static void blink_led_task(void* pvParameter) {
    configure_led();

    while (1) {
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, led_state);
        ESP_LOGI(TAG, "Led is %s", led_state ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
    }
}

void app_main(void) {
    // xTaskCreate(blink_led_task, "blink_task", 2048, NULL, 5,
    // &blink_task_handle);
    button_init(BUTTON_GPIO);

    while (1) {
        ESP_LOGI(TAG, "PING!");
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Button state is: %s", button_get_state());
        ESP_LOGI(TAG, "Button press count is: %s", get_button_press_count());
        // if (blink_task_handle != NULL) {
        //     vTaskDelete(blink_task_handle);
        //     blink_task_handle = NULL;
        // }
    }
}