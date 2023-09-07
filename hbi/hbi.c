#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hbi.pio.h"
#include "hbi.h"


void hbiProtocolHandler(void)
{ 
    static int state = 0; //State of the handler in case of multi byte commands. Only declared to be 0 on initialization.
    extern struct hbi mainLink;
    uint32_t data = pio_sm_get(mainLink.hbiPIO, mainLink.RXsm);

    switch (data)
    {
    case 1: //Ping, read next byte and send it back
        pio_sm_put(mainLink.hbiPIO, mainLink.TXsm, pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm));
        break;
    
    case 2: //MemPut, put a variable in memory
        uint16_t index;
        uint16_t size;
        index = (uint16_t)(mainLink.hbiPIO, mainLink.RXsm);
        index = ((uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm) << 8) | index; //shift for the full 16 bit index
        
        size = (uint16_t)(mainLink.hbiPIO, mainLink.RXsm);
        size = ((uint16_t)pio_sm_get_blocking(mainLink.hbiPIO, mainLink.RXsm) << 8) | size;
        break;
    
    default:
        break;
    } 
}


void start_hbi(PIO hbiPIO, uint pinBase, struct hbi *currentHbi) //Start a new high bandwidth interconnect on hbiPIO starting at pinBase
{ 
    currentHbi->hbiPIO = hbiPIO;
    uint irqNum;
    const int interruptSource[] = {pis_sm0_rx_fifo_not_empty, pis_sm1_rx_fifo_not_empty, pis_sm2_rx_fifo_not_empty, pis_sm3_rx_fifo_not_empty};
    //Array of the interrupt source since we aren't specifying the state machine we are using

    currentHbi->offsetTX = pio_add_program(currentHbi->hbiPIO, &hbiTX_program); //Add pio programs to pio execution memory
    currentHbi->offsetRX = pio_add_program(currentHbi->hbiPIO, &hbiRX_program);

    currentHbi->TXsm = pio_claim_unused_sm(hbiPIO, true); //Get state machines
    currentHbi->RXsm = pio_claim_unused_sm(hbiPIO, true);

    pio_set_irq0_source_enabled(currentHbi->hbiPIO, interruptSource[currentHbi->RXsm], true); //Since 
    
    if (currentHbi->hbiPIO == pio0) //IRQ num changes depending on which PIO you are using, thus this code
        irqNum = 7;
     else 
        irqNum = 9;
    
    irq_set_exclusive_handler(irqNum, hbiProtocolHandler);
    irq_set_enabled(irqNum, true);

    init_hbiTX(currentHbi->hbiPIO, currentHbi->TXsm, pinBase, pinBase + 4, currentHbi->offsetTX); //Start hbi (functions defined in the hbi.pio file)
    init_hbiRX(currentHbi->hbiPIO, currentHbi->RXsm, pinBase + 5, pinBase + 11, currentHbi->offsetRX);
}

