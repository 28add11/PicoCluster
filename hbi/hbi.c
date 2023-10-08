#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hbi.pio.h"
#include "hbi.h"
#include <stdint.h>
#include <stdio.h>


void hbiProtocolHandler(void) { 

	gpio_put(25, 1);
    static int state = 0; //State of the handler in case of multi byte commands. Only declared to be 0 on initialization.
    extern struct hbi mainLink;
	irq_clear(mainLink.irqNum);

    uint32_t data = pio_sm_get(mainLink.hbiPIO, mainLink.RXsm);

    switch (data){
    case 1: //Ping, read next byte and send it back
        pio_sm_put(mainLink.hbiPIO, mainLink.TXsm, pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm));
        break;
    
    case 2: {//MemPut, put a variable in memory
    	    uint16_t index;
    		uint16_t size;
	
    		index = (uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm); // Assign the value to index
    		index = ((uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm) << 8) | index; //shift for the full 16 bit index
	
    		size = (uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm); // Assign the value to size
    		size = (uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm) | size;
		}
    	break;
    
    default:
        break;
    } 

	uint64_t timeStart = time_us_64(); // Busy wait code
	while ((time_us_64() - timeStart) < 50000)
	{
		tight_loop_contents;
	}
	
	gpio_put(25, 0);
}


void start_hbi(PIO hbiPIO, uint pinBase, struct hbi *currentHbi) { //Start a new high bandwidth interconnect on hbiPIO starting at pinBase

    currentHbi->hbiPIO = hbiPIO;
    uint irqNum;
    const int interruptSource[] = {pis_sm0_rx_fifo_not_empty, pis_sm1_rx_fifo_not_empty, pis_sm2_rx_fifo_not_empty, pis_sm3_rx_fifo_not_empty};
    //Array of the interrupt source since we aren't specifying the state machine we are using

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    currentHbi->offsetTX = pio_add_program(currentHbi->hbiPIO, &hbiTX_program); //Add pio programs to pio execution memory
    currentHbi->offsetRX = pio_add_program(currentHbi->hbiPIO, &hbiRX_program);

    currentHbi->TXsm = pio_claim_unused_sm(hbiPIO, true); //Get state machines
    currentHbi->RXsm = pio_claim_unused_sm(hbiPIO, true);

    pio_set_irq0_source_enabled(currentHbi->hbiPIO, currentHbi->irqNum, true); 
    
    if (currentHbi->hbiPIO == pio0) // Since IRQ num changes depending on which PIO you are using, thus this code
        currentHbi->irqNum = 7;
     else 
        currentHbi->irqNum = 9;
    
    irq_set_exclusive_handler(currentHbi->irqNum, hbiProtocolHandler);
    irq_set_enabled(currentHbi->irqNum, true);

    init_hbiTX(currentHbi->hbiPIO, currentHbi->TXsm, pinBase, pinBase + 5, currentHbi->offsetTX); //Start hbi (functions defined in the hbi.pio file)
    init_hbiRX(currentHbi->hbiPIO, currentHbi->RXsm, pinBase + 6, pinBase + 12, currentHbi->offsetRX);
}

uint32_t sendPing(struct hbi *interconnect, uint8_t data) {
	pio_sm_put(interconnect->hbiPIO, interconnect->TXsm, (uint32_t)0x01000000); // Hex constant for ping datatype
	// Remember that PIO out grabs from left when OSR shifts left
	irq_set_enabled(interconnect->irqNum, false); // Disable the IRQ handler to prevent IRQ during ping return

	uint32_t startTime = time_us_32();
	pio_sm_put(interconnect->hbiPIO, interconnect->TXsm, ((uint32_t)data) << 24);
	pio_sm_get_blocking(interconnect->hbiPIO, interconnect->TXsm);
	uint32_t timeDif = time_us_32() - startTime; // Calculate this here instead of in the return to make sure irq_set_enabled call doesnt affect ping time

	irq_set_enabled(interconnect->irqNum, true);
	return timeDif;
}