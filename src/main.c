#include "hardware/structs/rosc.h"

#include <string.h>
#include <time.h>

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
#include "mqtt.h"
#include "wifi.h"

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

  if (!wifi_connect()) {
    return 1;
  }

  MQTT_APP_STATE_T *state = get_mqtt_state();

  dns_lookup(state);

  app(state);

  cyw43_arch_deinit();
  return 0;
}