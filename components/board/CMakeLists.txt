cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS 
        "board.cpp"
        "board_display.cpp"
        "board_wifi.cpp"
    INCLUDE_DIRS
        "."
        "interface"
    REQUIRES
        modules
    PRIV_REQUIRES
        devices
        esp_wifi
)