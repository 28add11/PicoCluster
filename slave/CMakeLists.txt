add_executable(picoClusterSlave
        slave.c
        )

#pico_set_binary_type(picoClusterSlave copy_to_ram)

target_link_libraries(picoClusterSlave pico_stdlib hardware_spi hardware_watchdog)
pico_add_extra_outputs(picoClusterSlave)
