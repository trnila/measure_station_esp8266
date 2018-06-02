#pragma once
#include <paho_mqtt_c/MQTTClient.h>
#include "config.h"

void mqtt_init();
void publish(const char* topic, const int retained, const char* msg_fmt, ...);
// register callback only in initialization
void subscribe(const char *topic, void (*callback)(mqtt_message_data_t *));
void mqtt_task(void *pvParameters);

#define MQTT_TOPIC(topic) MQTT_BASE_TOPIC topic
