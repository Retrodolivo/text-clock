cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS 
        "application.cpp"
    INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        system
        modules
)