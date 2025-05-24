cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS
        "nettime/nettime.cpp"
    INCLUDE_DIRS 
        "color"
        "nettime"
    PRIV_REQUIRES
        lwip
        esp_netif
)