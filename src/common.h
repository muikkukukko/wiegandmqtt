#ifndef WIEGANDMQTT_COMMON
#define WIEGANDMQTT_COMMON

#include "lwip/api.h"
#include "lwip/apps/mqtt.h"

#define DEBUG_PRINT printf

typedef struct MQTT_APP_STATE_T_ {
  ip_addr_t remote_addr;
  mqtt_client_t *mqtt_client;
  u32_t received;
  u32_t counter;
  u32_t reconnect;
} MQTT_APP_STATE_T;

typedef enum { NO_CODE, KEYPAD, CARD } code_types_t;

#endif /* WIEGANDMQTT_COMMON */
