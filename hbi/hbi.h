#pragma once

struct hbi {
    PIO hbiPIO;
    uint offsetTX;
    uint offsetRX;
    uint TXsm;
    uint RXsm;
	uint irqNum;
};

/*! \brief Initialize a connection using the custom HBI 
    
    \param hbiPIO The PIO you want this connection to be on
    \param pinBase The first pin for the connection (12 pins total)
    \param currentHbi The hbi struct where the information from this will go
*/
void start_hbi(PIO hbiPIO, uint pinBase, struct hbi *currentHbi);

void *sendReq(struct hbi interconnect, uint reqType, void *data);