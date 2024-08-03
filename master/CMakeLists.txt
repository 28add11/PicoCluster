add_executable(picoClusterMaster
        master.c
		interface.c
        )

target_link_libraries(picoClusterMaster PRIVATE pico_stdlib pico_rand hardware_spi)
pico_add_extra_outputs(picoClusterMaster)
