#ifndef WIEGANDMQTT_MQTT
#define WIEGANDMQTT_MQTT

#include "common.h"

MQTT_APP_STATE_T *get_mqtt_state(void);

err_t mqtt_connect(MQTT_APP_STATE_T *state);

err_t mqtt_msg_publish(MQTT_APP_STATE_T *state);

#endif /* WIEGANDMQTT_MQTT */
