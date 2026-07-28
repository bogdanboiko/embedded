#include <stdio.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display/display_driver.h"
#include "encoder/encoder_driver.h"

// display
#define I2C_SDA_PORT GPIO_NUM_15
#define I2C_SCL_PORT GPIO_NUM_16

// encoder
#define CN_A_GPIO GPIO_NUM_4
#define CN_B_GPIO GPIO_NUM_5
#define CN_SW_GPIO GPIO_NUM_6

static const char* TAG = "OLED display";

void app_main(void) {
    config_encoder(CN_A_GPIO, CN_B_GPIO, CN_SW_GPIO);
    config_device_protocol(I2C_SDA_PORT, I2C_SCL_PORT);
    bool is_display_connected = troubleshoot_display_connection_success();

    ESP_LOGI(TAG, "Display connection status is %s!",
             is_display_connected ? "successful" : "failed");

    if (is_display_connected) {
        config_display();
    }

    clear_display_buffer();
    canvas_point_t start = {.x = 0, .y = 0 };
    canvas_point_t end = {.x = 90, .y = 55 };
    draw_line(start, end);
    // canvas_point_t top_left = {.x = 52, .y = 17};
    // canvas_point_t right_bottom = {.x = 90, .y = 45};
    // draw_rect(top_left, right_bottom);
    


    // draw_line((canvas_point_t) {.x = 12, .y = 12}, (canvas_point_t) {.x = 90, .y = 55});
    // draw_line((canvas_point_t) {.x = 12, .y = 55}, (canvas_point_t) {.x = 90, .y = 12});

    int current_count;
    int current_angle;

    while (1) {
        current_count = get_current_count();
        current_angle = get_current_angle();

        ESP_LOGI(TAG, "Interrup fired, new count value is: %d, angle is: %d", current_count, current_angle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
