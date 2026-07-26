idf_component_register(SRCS "main.c" "display_driver.c"
                       PRIV_REQUIRES esp_driver_i2c esp_driver_gpio
                       INCLUDE_DIRS ".")
