#ifndef WIEGANDMQTT_DMA
#define WIEGANDMQTT_DMA

#include "hardware/pio.h"

#define CARD_TOTAL_BITS 58
#define NUM_OF_BITS_IN_DMA 2
#define DMA_BITS_MASK 3
#define FULL_BYTE 8

void dma_init(int dma_channel, PIO pio, uint sm);

#endif /* WIEGANDMQTT_DMA */
