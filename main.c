#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hbi/hbi.h"

struct hbi mainLink;

int main(void) {
    stdio_init_all();

    start_hbi(pio0, 2, &mainLink); //start at pin 2 in case we want to debug with UART

	uint32_t pingData = 69;

	printf("pingTime: %i\n", *sendReq(mainLink, 1, &pingData));
    return 0;
}