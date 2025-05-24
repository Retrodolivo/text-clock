cmake_minimum_required(VERSION 3.16)

set(APPLICATION_DIR ../application)

idf_component_register(
    SRCS
        "main.cpp" 
        "system.cpp"
        "${APPLICATION_DIR}/application.cpp"
    INCLUDE_DIRS
        "."
        "${APPLICATION_DIR}"
    PRIV_REQUIRES
        board
        nvs_flash
)