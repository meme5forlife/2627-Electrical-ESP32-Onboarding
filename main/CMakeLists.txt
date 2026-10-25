idf_component_register(SRCS "main.c"
                            "src/bno055.c"
                            "src/ultrasonic.c"
                            "src/wheel.c"
                       INCLUDE_DIRS "."
                                    "inc"
                                    "src"
                       PRIV_REQUIRES spi_flash 
                                     esp_timer 
                                     esp_driver_gpio 
                                     esp_driver_uart 
                                     esp_driver_ledc 
                                     esp_driver_i2c)