/* Photo ADC — читання значення з фоторезистора у консоль */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "led_controller.h"

static const char *TAG = "photo_adc";

#define LED_GPIO 35

#define PHOTO_ADC_UNIT     ADC_UNIT_1
#define PHOTO_ADC_CHANNEL  ADC_CHANNEL_4      // GPIO 5 на ESP32-S3
#define PHOTO_ADC_ATTEN    ADC_ATTEN_DB_12    // повний діапазон ~0..3.1 В
#define PHOTO_ADC_BITWIDTH ADC_BITWIDTH_12

static adc_oneshot_unit_handle_t s_adc;

static void configure_adc(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = PHOTO_ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_cfg, &s_adc);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = PHOTO_ADC_ATTEN,
        .bitwidth = PHOTO_ADC_BITWIDTH,
    };
    adc_oneshot_config_channel(s_adc, PHOTO_ADC_CHANNEL, &chan_cfg);
}

void app_main(void)
{
    config_led_pwm(LED_GPIO);
    configure_adc();
    ESP_LOGI(TAG, "Photoresistor on ADC1_CH%d (GPIO5)", PHOTO_ADC_CHANNEL);

    while (1) {
        int raw = 0;

        adc_oneshot_read(s_adc, PHOTO_ADC_CHANNEL, &raw);
        float maxLitValue = (1 << PHOTO_ADC_BITWIDTH);
        float litPercent = raw / maxLitValue;
        ESP_LOGI(TAG, "Room light level %f, max: %f", litPercent, maxLitValue);
        set_brightness_level(1 - litPercent);

        ESP_LOGI(TAG, "raw = %d", raw);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}