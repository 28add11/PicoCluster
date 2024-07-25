cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(picoCluster)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
#add_subdirectory(hbi)

add_executable(picoClusterMaster
        main.c
        )

add_executable(picoClusterSlave
        main.c
        )

target_compile_definitions(picoClusterMaster PRIVATE MASTER=1)
target_compile_definitions(picoClusterSlave PRIVATE MASTER=0)

# pull in common dependencies
target_link_libraries(picoClusterMaster PRIVATE pico_stdlib hardware_spi)
target_link_libraries(picoClusterSlave PRIVATE pico_stdlib hardware_spi)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoClusterMaster)
pico_add_extra_outputs(picoClusterSlave)