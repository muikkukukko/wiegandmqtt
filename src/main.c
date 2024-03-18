#include "hardware/structs/rosc.h"

#include <string.h>
#include <time.h>

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"

#include "common.h"
#include "dma.h"
#include "mqtt.h"
#include "wifi.h"

#include "wiegand.pio.h"

#define IO_BASE_PIN 2

// #include "tusb.h"

void app(MQTT_APP_STATE_T *state) {

  state->counter = 0;

  if (mqtt_connect(state) == ERR_OK) {
    absolute_time_t timeout = nil_time;

    while (true) {
      cyw43_arch_poll();
      absolute_time_t now = get_absolute_time();
      if (is_nil_time(timeout) || absolute_time_diff_us(now, timeout) <= 0) {
        if (mqtt_client_is_connected(state->mqtt_client)) {
          cyw43_arch_lwip_begin();

          if (mqtt_msg_publish(state) == ERR_OK) {
            if (state->counter != 0) {
              DEBUG_PRINT("published %d\n", state->counter);
            }
            timeout = make_timeout_time_ms(5000);
            state->counter++;
          } // else ringbuffer is full and we need to wait for messages to
            // flush.
          cyw43_arch_lwip_end();
        } else {
          // DEBUG_printf(".");
        }
      }
    }
  }
}

int main() {
  stdio_init_all();

  gpio_init(IO_BASE_PIN);
  gpio_init(IO_BASE_PIN + 1);
  gpio_set_dir(IO_BASE_PIN, false);
  gpio_set_dir(IO_BASE_PIN + 1, false);

  PIO pio = pio0;
  uint offset = pio_add_program(pio, &wiegand_program);
  uint sm = pio_claim_unused_sm(pio, true);
  wiegand_program_init(pio, sm, offset, IO_BASE_PIN);

  if (!wifi_connect()) {
    return 1;
  }

  int wiegand_dma_ch = dma_claim_unused_channel(false);
  if (wiegand_dma_ch >= 0) {
    DEBUG_PRINT("Claimed dma ch %d for Wiegand\n", wiegand_dma_ch);
    dma_init(wiegand_dma_ch, pio, sm);
  } else {
    DEBUG_PRINT("Unable to claim dma channel for Wiegand\n");
  }

  // MQTT_APP_STATE_T *state = get_mqtt_state();

  // dns_lookup(state);

  // app(state);
  while (true) {
  }

  cyw43_arch_deinit();
  return 0;
}