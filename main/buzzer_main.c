#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"
#include "buzzer.h"

static const char* TAG = "example";

#define BUZZER_GPIO 35
#define BUTTON_GPIO 11  

static TaskHandle_t buzzer_task_handler = NULL;

static void buzzer_task(void* arg) {
    buzzer_set_volume(0.5);

    for (int i = 0; i < 4; i++) {
        buzzer_set_pitch(400);
        vTaskDelay(pdMS_TO_TICKS(150));
        buzzer_set_pitch(800);
        vTaskDelay(pdMS_TO_TICKS(150));
    }

    buzzer_set_volume(0);
    
    buzzer_task_handler = NULL;
    vTaskDelete(NULL);
}

void app_main(void) {
    button_init(BUTTON_GPIO);
    buzzer_pwm_init(BUZZER_GPIO);

    while (1) {
        bool isButtonPressed = button_get_state();

        if (isButtonPressed) {
            ESP_LOGI(TAG, "Button was pressed!");
            if (buzzer_task_handler != NULL) {
                vTaskDelete(buzzer_task_handler);
                buzzer_task_handler = NULL;
            }

            xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 10,
                        &buzzer_task_handler);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
