cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS
        "system.cpp"
        "services/sys_service_application.cpp"
    INCLUDE_DIRS
        "."
        "interface"
        "services"
    PRIV_REQUIRES
        application
        board
        modules
)

if(DEFINED ENV{MY_WIFI_SSID} AND DEFINED ENV{MY_WIFI_PASSWD})
    target_compile_definitions(${COMPONENT_LIB}
        PUBLIC MY_WIFI_SSID="$ENV{MY_WIFI_SSID}"
               MY_WIFI_PASSWD="$ENV{MY_WIFI_PASSWD}"
    )
else()
    message(FATAL_ERROR "Please set MY_WIFI_SSID and MY_WIFI_PASSWD environment variables")
endif()