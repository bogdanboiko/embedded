idf_component_register(SRCS "main.c" "button.c" "ledc_controller.c" "state_machine.c"
PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_pm
                       INCLUDE_DIRS ".")
