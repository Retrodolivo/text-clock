cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS 
        "application.cpp"
        "render/render.cpp"
    INCLUDE_DIRS
        "."
        "render"
    PRIV_REQUIRES
        system
        modules
)