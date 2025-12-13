cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS 
       "board_clock.cpp"
       "board_display.cpp"
       "board_wifi.cpp"
       "board_info.cpp"
    INCLUDE_DIRS
        "."
        "devices/addressable_led"
    PRIV_REQUIRES
        system
        modules
        esp_hw_support
        esp_driver_rmt
        esp_wifi
        nvs_flash
)