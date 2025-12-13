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
        modules
)