cmake_minimum_required(VERSION 3.13)
# rest of your project

add_library(hbiLib
        hbi.c
        )

# pull in common dependencies
target_link_libraries(hbiLib pico_stdlib hardware_pio hardware_irq)