cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS
        "main.cpp" 
        "app_system_init.cpp"
        "../application/application.cpp"
    INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        board
)