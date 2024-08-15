/*
//	Code for "Slave" pico in cluster
//
//	Written by Nicholas West, 2024
//  Some code taken from https://forums.raspberrypi.com/viewtopic.php?t=359490 (Mostly for fixing RAM jumps from watchdog reboots)
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"
//#include "hbi/hbi.h"
#include <stdio.h>
#include <stdlib.h>

#include "slave.h"


// The following code is to fix an issue with sram clock setup on boots, and is from Raspberry Pi. Their license is below.

/*
Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
   disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Includes specifically for this portion (boot stuff is soooo fun)
#include "hardware/structs/rosc.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/xosc.h"
#include "hardware/structs/pll.h"

static void _usb_clock_setup() {
    // First make absolutely sure clk_ref is running: needed for resuscitate,
    // and to run clk_sys while configuring sys PLL. Assume that rosc is not
    // configured to run faster than clk_sys max (as this is officially out of
    // spec)
    // If user previously configured clk_ref to a different source (e.g.
    // GPINx), then halted that source, the glitchless mux can't switch away
    // from the dead source-- nothing we can do about this here.
    rosc_hw->ctrl = ROSC_CTRL_ENABLE_VALUE_ENABLE << ROSC_CTRL_ENABLE_LSB;
    hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);

    // Resuscitate logic will switch clk_sys to clk_ref if it is inadvertently stopped
    clocks_hw->resus.ctrl =
            CLOCKS_CLK_SYS_RESUS_CTRL_ENABLE_BITS |
            (CLOCKS_CLK_SYS_RESUS_CTRL_TIMEOUT_RESET
                    << CLOCKS_CLK_SYS_RESUS_CTRL_TIMEOUT_LSB);

    // Resetting PLL regs or changing XOSC range can glitch output, so switch
    // clk_sys away before touching. Not worried about clk_usb as USB is held
    // in reset.
    hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    while (!(clocks_hw->clk[clk_sys].selected & 1u));
    // rosc can not (while following spec) run faster than clk_sys max, so
    // it's safe now to clear dividers in clkslices.
    clocks_hw->clk[clk_sys].div = 0x100; // int 1 frac 0
    clocks_hw->clk[clk_usb].div = 0x100;

    // Try to get the crystal running. If no crystal is present, XI should be
    // grounded, so STABLE counter will never complete. Poor designs might
    // leave XI floating, in which case we may eventually drop through... in
    // this case we rely on PLL not locking, and/or resuscitate counter.
    //
    // Don't touch range setting: user would only have changed if crystal
    // needs it, and running crystal out of range can produce glitchy output.
    // Note writing a "bad" value (non-aax) to RANGE has no effect.
    xosc_hw->ctrl = XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB;
    while (!(xosc_hw->status & XOSC_STATUS_STABLE_BITS));

    // Sys PLL setup:
    // - VCO freq 1200 MHz, so feedback divisor of 100. Range is 400 MHz to 1.6 GHz
    // - Postdiv1 of 5, down to 240 MHz (appnote recommends postdiv1 >= postdiv2)
    // - Postdiv2 of 5, down to 48 MHz
    //
    // Total postdiv of 25 means that too-fast xtal will push VCO out of
    // lockable range *before* clk_sys goes out of closure (factor of 1.88)
    reset_block(RESETS_RESET_PLL_SYS_BITS);
    unreset_block_wait(RESETS_RESET_PLL_SYS_BITS);
    pll_sys_hw->cs = 1u << PLL_CS_REFDIV_LSB;
    pll_sys_hw->fbdiv_int = 100;
    pll_sys_hw->prim =
            (5u << PLL_PRIM_POSTDIV1_LSB) |
            (5u << PLL_PRIM_POSTDIV2_LSB);

    // Power up VCO, wait for lock
    hw_clear_bits(&pll_sys_hw->pwr, PLL_PWR_PD_BITS | PLL_PWR_VCOPD_BITS);
    while (!(pll_sys_hw->cs & PLL_CS_LOCK_BITS));

    // Power up post-dividers, which ungates PLL final output
    hw_clear_bits(&pll_sys_hw->pwr, PLL_PWR_POSTDIVPD_BITS);

    // Glitchy switch of clk_usb, clk_sys aux to sys PLL output.
    clocks_hw->clk[clk_sys].ctrl = 0;
    clocks_hw->clk[clk_usb].ctrl =
            CLOCKS_CLK_USB_CTRL_ENABLE_BITS |
            (CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS
                    << CLOCKS_CLK_USB_CTRL_AUXSRC_LSB);

    // Glitchless switch of clk_sys to aux source (sys PLL)
    hw_set_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    while (!(clocks_hw->clk[clk_sys].selected & 0x2u));
}


// This code is from adam_green on the rpi forums, many thanks to them!
#include "pico/bootrom.h"
extern uint32_t aeabi_mem_funcs[4];
extern uint32_t aeabi_bits_funcs[4];
static void reinitFunctionTables()
{
    // Before running reset again, re-initialize the function tables with the required Boot ROM codes rather than the
    // function pointer values they currently hold from the last reset.
    aeabi_mem_funcs[0] = ROM_FUNC_MEMSET;
    aeabi_mem_funcs[1] = ROM_FUNC_MEMCPY;
    aeabi_mem_funcs[2] =  ROM_FUNC_MEMSET4;
    aeabi_mem_funcs[3] =  ROM_FUNC_MEMCPY44;

    aeabi_bits_funcs[0] = ROM_FUNC_POPCOUNT32;
    aeabi_bits_funcs[1] = ROM_FUNC_CLZ32;
    aeabi_bits_funcs[2] = ROM_FUNC_CTZ32;
    aeabi_bits_funcs[3] = ROM_FUNC_REVERSE32;
}

int main(void) {
    stdio_init_all();

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);

	printf("started\n");

	spi_init(spi0, 500000);

	spi_set_slave(spi0, true);

	spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);
	
	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);
	gpio_set_function(6, GPIO_FUNC_SPI);
	gpio_set_function(7, GPIO_FUNC_SPI);

	gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
	gpio_put(21, 0);

	uint8_t value[2];

	//__breakpoint();

	while (1) {

		gpio_put(21, 0);
		
		spi_read_blocking(spi0, 0xFF, value, 2);

		//printf("Instruction:\t%i, %i\n", value[0], value[1]);

		switch (value[0]) {

		case 1: // Reset (only reset SPI block because we are stateless beond single instructions)
			
			reset_block(RESETS_RESET_SPI0_BITS); // Actually reset the block
			gpio_put(LED_PIN, 1);
			unreset_block_wait(RESETS_RESET_SPI0_BITS); // Also serves as a delay so LED can actually do stuff
			gpio_put(LED_PIN, 0);
			gpio_put(21, 1);

			spi_init(spi0, 500000);
			spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);
			spi_set_slave(spi0, true);
			break;
			
		case 2: // Ping
			gpio_put(21, 1);
			spi_write_blocking(spi0, &value[1], 1);
			break;

		case 3: // Memory access
			;
			size_t size;
			uint8_t *address;
			uint8_t data;

			switch (value[1]) { // Get what we are actually doing
			case 0: // Read
				// Get adress to read from
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&address), sizeof(uint8_t *));

				data = *address;
				
				spi_write_blocking(spi0, &data, 1);
				break;

			case 1: // Write
				// Get adress to write to
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&address), sizeof(uint8_t *));
				
				
				spi_read_blocking(spi0, 0xFF, &data, 1);

				*address = data;
				break;

			case 2: // Malloc
				// Get the size of the buffer to allocate
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&size), sizeof(size_t));

				address = malloc(size);
				
				spi_write_blocking(spi0, (uint8_t *)(&address), sizeof(uint8_t *));
				break;
			
			default:
				break;
			}
			break;
		
		case 4: // Program context switch
			;
			// Get address of where to start running (program type defined in slave.h)
			program executable;
			
			spi_read_blocking(spi0, 0xFF, (uint8_t *)(&executable), sizeof(int *));

			printf("Balling: starting execution\n\n");

			// Transfer execution
			sleep_ms(20);
			executable = (program)((int)(executable) | 1); // | 1 relates to ARM's thumb execution, which is required for execution to work
			_usb_clock_setup(); // Required due to issues with SRAM clocks
			reinitFunctionTables(); // Required due to issues with warm boots
			watchdog_reboot(0x20004001, 0x20042000, 0x7FFFFF);
			break;

		default:
			break;
		}
	}

    return 0;
}