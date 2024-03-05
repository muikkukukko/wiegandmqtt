#ifndef WIEGANDMQTT_WIFI
#define WIEGANDMQTT_WIFI

#include "common.h"

bool wifi_connect(void);
void dns_lookup(MQTT_APP_STATE_T *state);

#endif /* WIEGANDMQTT_WIFI */
