# Add blinkLED.c and create the binary
add_executable(blinkLED blink.c)

target_link_libraries(blinkLED PRIVATE pico_stdlib)

# Ensure the generated directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)

# Custom command to convert the ELF binary into a raw binary file
add_custom_command(
    OUTPUT blinkLED.bin
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:blinkLED> blinkLED.bin
    DEPENDS blinkLED
    COMMENT "Generating raw binary from blinkLED"
)

# Custom command to convert the raw binary file into a C header file
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/generated/blinkLED_bin.h
    COMMAND xxd -i blinkLED.bin > ${CMAKE_BINARY_DIR}/generated/blinkLED_bin.h
    DEPENDS blinkLED.bin
    COMMENT "Generating binary header from blinkLED"
)

# Add custom targets to drive the custom commands
add_custom_target(generate_raw_binary ALL DEPENDS blinkLED.bin)
add_custom_target(embed_blinkLED_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/generated/blinkLED_bin.h)
