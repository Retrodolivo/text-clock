cmake_minimum_required(VERSION 3.16)

idf_component_register(
    INCLUDE_DIRS 
        "addressable_led"
    REQUIRES 
        modules
        esp_driver_rmt
    PRIV_REQUIRES
        esp_hw_support
)
