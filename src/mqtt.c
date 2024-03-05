#include <string.h>

#include "pico/cyw43_arch.h"

#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"

#include "common.h"
#include "mqttsettings.h"

#define PUB_TOPIC "id/read"
#define SUB_TOPIC "id/messages"

static const char *cert = CRYPTO_CERT;
static const char *ca = CRYPTO_CA;
static const char *key = CRYPTO_KEY;

MQTT_APP_STATE_T state_instance;
mqtt_client_t mqtt_client;

u32_t data_in = 0;

u8_t buffer[1025];
u8_t data_len = 0;

static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
  DEBUG_PRINT("mqtt_pub_start_cb: topic %s\n", topic);

  if (tot_len > 1024) {
    DEBUG_PRINT("Message length exceeds buffer size, discarding");
  } else {
    data_in = tot_len;
    data_len = 0;
  }
}

static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len,
                             u8_t flags) {
  if (data_in > 0) {
    data_in -= len;
    memcpy(&buffer[data_len], data, len);
    data_len += len;

    if (data_in == 0) {
      buffer[data_len] = 0;
      DEBUG_PRINT("Message received: %s\n", &buffer);
    }
  }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
  MQTT_APP_STATE_T *state = (MQTT_APP_STATE_T *)arg;
  DEBUG_PRINT("mqtt_pub_request_cb: err %d\n", err);
  state->received++;
}

void mqtt_sub_request_cb(void *arg, err_t err) {
  DEBUG_PRINT("mqtt_sub_request_cb: err %d\n", err);
}

err_t mqtt_msg_publish(MQTT_APP_STATE_T *state) {
  char buffer[128];

  sprintf(buffer, "{\"message\":\"hello from picow %d / %d\"}", state->received,
          state->counter);

  err_t err;
  u8_t qos = 0;
  u8_t retain = 0;
  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, PUB_TOPIC, buffer, strlen(buffer), qos,
                     retain, mqtt_pub_request_cb, state);
  cyw43_arch_lwip_end();
  if (err != ERR_OK) {
    DEBUG_PRINT("Publish err: %d\n", err);
  }

  return err;
}

MQTT_APP_STATE_T *get_mqtt_state(void) {
  state_instance.mqtt_client = &mqtt_client;
  return &state_instance;
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                               mqtt_connection_status_t status) {

  MQTT_APP_STATE_T *state = (MQTT_APP_STATE_T *)arg;

  if (status != 0) {
    DEBUG_PRINT("Error during connection: err %d.\n", status);
  } else {
    cyw43_arch_lwip_begin();
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb,
                            mqtt_pub_data_cb, 0);
    mqtt_sub_unsub(state->mqtt_client, SUB_TOPIC, 0, mqtt_sub_request_cb, 0, 1);
    cyw43_arch_lwip_end();
    DEBUG_PRINT("MQTT connected.\n");
  }
}

err_t mqtt_connect(MQTT_APP_STATE_T *state) {
  struct mqtt_connect_client_info_t ci;
  err_t err;

  memset(&ci, 0, sizeof(ci));

  ci.client_id = "PicoW";
  ci.client_user = NULL;
  ci.client_pass = NULL;
  ci.keep_alive = 0;
  ci.will_topic = NULL;
  ci.will_msg = NULL;
  ci.will_retain = 0;
  ci.will_qos = 0;

  struct altcp_tls_config *tls_config;

  DEBUG_PRINT("Setting up TLS\n");
  tls_config = altcp_tls_create_config_client_2wayauth(
      (const u8_t *)ca, 1 + strlen((const char *)ca), (const u8_t *)key,
      1 + strlen((const char *)key), (const u8_t *)"", 0, (const u8_t *)cert,
      1 + strlen((const char *)cert));

  if (tls_config == NULL) {
    DEBUG_PRINT("Failed to initialize config\n");
    return -1;
  }

  ci.tls_config = tls_config;

  const struct mqtt_connect_client_info_t *client_info = &ci;

  err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr),
                            MQTT_SERVER_PORT, mqtt_connection_cb, state,
                            client_info);

  if (err != ERR_OK) {
    DEBUG_PRINT("mqtt_connect return %d\n", err);
  } else {
  }

  return err;
}
