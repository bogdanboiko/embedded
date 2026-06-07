idf_component_register(SRCS "button.c" "buzzer.c" "buzzer_main.c"
PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_pm
                       INCLUDE_DIRS ".")
