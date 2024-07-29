add_executable(picoClusterSlave
        slave.c
        )

target_link_libraries(picoClusterSlave PRIVATE pico_stdlib hardware_spi)
pico_add_extra_outputs(picoClusterSlave)
