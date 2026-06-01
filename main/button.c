#include "button.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "button";

static int s_gpio_num = -1; 
static volatile uint32_t s_press_count = 0;
static volatile bool s_button_state = false;

static void button_task(void *arg){
    while(1){
        bool current_state = gpio_get_level(s_gpio_num);
        
        if(current_state != s_button_state){
            s_button_state = current_state;
            if(s_button_state){
                s_press_count++;
                ESP_LOGI(TAG, "Button pressed! Total count: %d", s_press_count);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
    }
}

void button_init(int gpio_num) {
    s_gpio_num = gpio_num;
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << s_gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
}

bool button_get_state(void){
    return s_button_state;
}

uint32_t get_button_press_count(void){
    return s_press_count;
}