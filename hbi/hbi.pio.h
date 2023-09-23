// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ----- //
// hbiTX //
// ----- //

#define hbiTX_wrap_target 0
#define hbiTX_wrap 2

static const uint16_t hbiTX_program_instructions[] = {
            //     .wrap_target
    0x7004, //  0: out    pins, 4         side 0     
    0x38a4, //  1: wait   1 pin, 4        side 1     
    0x2024, //  2: wait   0 pin, 4                   
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program hbiTX_program = {
    .instructions = hbiTX_program_instructions,
    .length = 3,
    .origin = -1,
};

static inline pio_sm_config hbiTX_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + hbiTX_wrap_target, offset + hbiTX_wrap);
    sm_config_set_sideset(&c, 2, true, false);
    return c;
}

static inline void init_hbiTX(PIO pio, uint sm, uint pin_base, uint clk_pin, uint offset) {
    pio_sm_config c = hbiTX_program_get_default_config(offset);
    sm_config_set_out_pins(&c, pin_base, 4); //set up pins
    sm_config_set_in_pins (&c, pin_base + 4); 
    sm_config_set_sideset_pins (&c, clk_pin); //clk_pin can be anything, as opposed to being sequential to data as in hbiRX
    pio_sm_set_consecutive_pindirs(pio, sm, pin_base, 4, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_base + 4, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm, clk_pin, 1, true);
    for (uint i = pin_base; i < pin_base + 5; ++i)
        pio_gpio_init(pio, i);
    pio_gpio_init(pio, clk_pin);
    sm_config_set_out_shift(&c, false, true, 8);
    sm_config_set_fifo_join (&c, PIO_FIFO_JOIN_TX);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    }

#endif

// ----- //
// hbiRX //
// ----- //

#define hbiRX_wrap_target 0
#define hbiRX_wrap 2

static const uint16_t hbiRX_program_instructions[] = {
            //     .wrap_target
    0x3824, //  0: wait   0 pin, 4        side 1     
    0x30a4, //  1: wait   1 pin, 4        side 0     
    0x4004, //  2: in     pins, 4                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program hbiRX_program = {
    .instructions = hbiRX_program_instructions,
    .length = 3,
    .origin = -1,
};

static inline pio_sm_config hbiRX_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + hbiRX_wrap_target, offset + hbiRX_wrap);
    sm_config_set_sideset(&c, 2, true, false);
    return c;
}

static inline void init_hbiRX(PIO pio, uint sm, uint pin_base, uint ack_pin, uint offset) {
    pio_sm_config c = hbiRX_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin_base); //setting up pins, 5th pin is clock
    sm_config_set_sideset_pins(&c, ack_pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_base, 5, false);
    pio_sm_set_consecutive_pindirs(pio, sm, ack_pin, 1, true);
    for (uint i = pin_base; i < pin_base + 5; ++i)
        pio_gpio_init(pio, i);
    pio_gpio_init(pio, ack_pin);
    sm_config_set_in_shift(&c, false, true, 8);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX); //Make fifo bigger for DMA
    pio_sm_init(pio, sm, offset, &c); //send 'er running
    pio_sm_set_enabled(pio, sm, true);
    }

#endif

