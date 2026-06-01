idf_component_register(SRCS "fruit.c" "main.c" "wifi_manager.c" "buzzer.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "index.html" "index.js"
                    REQUIRES esp_http_server driver esp_adc nvs_flash esp_wifi esp_driver_ledc esp_driver_gpio esp_netif freertos esp_event esp_timer esp_pm)

