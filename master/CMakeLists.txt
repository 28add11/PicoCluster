add_executable(picoClusterMaster
        master.c
        )

target_link_libraries(picoClusterMaster PRIVATE pico_stdlib hardware_spi)
pico_add_extra_outputs(picoClusterMaster)
