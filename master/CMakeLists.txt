add_executable(picoClusterMaster
        master.c
		interface.c
        )

add_subdirectory(subProgram)

add_dependencies(picoClusterMaster embed_blinkLED_bin)
include_directories(${CMAKE_BINARY_DIR}/generated)

target_link_libraries(picoClusterMaster pico_stdlib pico_rand hardware_spi)
pico_add_extra_outputs(picoClusterMaster)
