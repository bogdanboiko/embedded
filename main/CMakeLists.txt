idf_component_register(SRCS "main.c" "servo/servo_driver.c" "display/display_driver.c" "encoder/encoder_driver.c"
                       PRIV_REQUIRES esp_driver_i2c esp_driver_gpio esp_timer esp_driver_ledc esp_pm
                       INCLUDE_DIRS ".")
