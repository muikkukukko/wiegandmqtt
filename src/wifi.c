#include "pico/cyw43_arch.h"

#include "lwip/dns.h"

#include "common.h"
#include "mqttsettings.h"

bool wifi_connect(void) {

  if (cyw43_arch_init()) {
    DEBUG_PRINT("Failed to initialise wifi\n");
    return false;
  }

  cyw43_arch_enable_sta_mode();

  DEBUG_PRINT("Connecting to WiFi...\n");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    DEBUG_PRINT("Wifi connection failed!\n");
    return false;
  } else {
    DEBUG_PRINT("Connected!\n");
  }

  return true;
}

static void dns_found_cb(const char *name, const ip_addr_t *ipaddr,
                         void *callback_arg) {
  MQTT_APP_STATE_T *state = (MQTT_APP_STATE_T *)callback_arg;
  DEBUG_PRINT("DNS query finished with resolved addr of %s.\n",
              ip4addr_ntoa(ipaddr));
  state->remote_addr = *ipaddr;
}

void dns_lookup(MQTT_APP_STATE_T *state) {
  DEBUG_PRINT("Running DNS query for %s.\n", MQTT_SERVER_HOST);

  cyw43_arch_lwip_begin();
  err_t err = dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr),
                                dns_found_cb, state);
  cyw43_arch_lwip_end();

  if (err == ERR_ARG) {
    DEBUG_PRINT("failed to start DNS query\n");
    return;
  }

  if (err == ERR_OK) {
    DEBUG_PRINT("no lookup needed");
    return;
  }

  while (state->remote_addr.addr == 0) {
    cyw43_arch_poll();
    sleep_ms(1);
  }
}