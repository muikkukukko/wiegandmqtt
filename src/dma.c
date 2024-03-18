
#include <hardware/dma.h>
#include <pico/time.h>

#include "common.h"
#include "dma.h"

#define RX_BUF_SIZE 10
#define MAX_INTERBYTE_TIME_FOR_CARD (150 * 1000)
volatile int dma_chan = 0;

static void dma_handler() {

  static uint32_t dma_dest = 0;
  static uint32_t byte_construct = 0;
  static uint32_t bytes[RX_BUF_SIZE];
  static bool first_time = true;
  static code_types_t receiving_type = NO_CODE;
  static absolute_time_t last_byte_time;
  static uint32_t bits_received = 0;

  // Clear the interrupt request.
  dma_hw->ints1 = 1u << dma_chan;

  // Early exit in special case (init)
  if (first_time) {
    dma_channel_set_write_addr(dma_chan, &dma_dest, true);
    first_time = false;
    bits_received = 0;
    return;
  }

  // Make room for new bit pair
  byte_construct <<= NUM_OF_BITS_IN_DMA;
  // Save new bit pair as least significant bits
  byte_construct |= (dma_dest & DMA_BITS_MASK);
  // Bits received
  bits_received += NUM_OF_BITS_IN_DMA;

  DEBUG_PRINT("DMA %d\n", bits_received);

  if ((bits_received % FULL_BYTE == 0) ||
      ((bits_received == CARD_TOTAL_BITS) && (receiving_type == CARD))) {

    uint32_t byte_point = (bits_received / FULL_BYTE) - 1;

    // Prevent under and overflow
    if (byte_point >= RX_BUF_SIZE) {
      byte_point = RX_BUF_SIZE - 1;
    }

    // Save byte in to buffer
    bytes[byte_point] = byte_construct;

    // In case of second byte,
    // infer the type from byte-byte spacing
    // (8-bit bursts from keypad are coming irregularly
    //  and with long space in between)
    if (byte_point == 1) {
      uint32_t interbyte_time =
          absolute_time_diff_us(last_byte_time, get_absolute_time());

      receiving_type =
          interbyte_time > MAX_INTERBYTE_TIME_FOR_CARD ? KEYPAD : CARD;

      if (receiving_type == CARD) {
        DEBUG_PRINT("CARD\n");
      }
    }

    if ((receiving_type == KEYPAD) && (bytes[byte_point] == 0xE1)) {
      DEBUG_PRINT("KEYPAD ready\n");
      receiving_type = NO_CODE;
      bits_received = 0;
    } else if ((receiving_type == CARD) && (bits_received == 58)) {
      DEBUG_PRINT("CARD ready\n");
      receiving_type = NO_CODE;
      bits_received = 0;
    }

    // Prepare for new byte
    byte_construct = 0;

    last_byte_time = get_absolute_time();
  }

  // Re-trigger DMA channel
  dma_channel_set_write_addr(dma_chan, &dma_dest, true);
}

void dma_init(int dma_channel, PIO pio, uint sm) {
  // Save channel num for later use
  dma_chan = dma_channel;

  dma_channel_config c = dma_channel_get_default_config(dma_chan);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));

  dma_channel_configure(dma_chan, &c,
                        NULL,          // Destination pointer
                        &pio->rxf[sm], // Source pointer
                        1,             // Number of transfers
                        false          // Start immediately
  );

  dma_irqn_acknowledge_channel(0, dma_chan);
  dma_irqn_acknowledge_channel(1, dma_chan);
  dma_channel_set_irq1_enabled(dma_chan, true);

  irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);
  irq_set_enabled(DMA_IRQ_1, true);

  // Call handler once to get process going
  dma_handler();
}