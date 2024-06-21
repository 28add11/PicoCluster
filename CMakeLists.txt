cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(picoCluster)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
#add_subdirectory(hbi)

add_executable(picoCluster
        main.c
        )

# pull in common dependencies
target_link_libraries(picoCluster PRIVATE pico_stdlib hardware_spi hbiLib)




# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoCluster)