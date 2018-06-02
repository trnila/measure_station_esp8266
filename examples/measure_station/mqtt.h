#pragma once
#include "config.h"

void mqtt_init();
void publish(const char* topic, const char* msg_fmt, ...);
void mqtt_task(void *pvParameters);

#define MQTT_TOPIC(topic) MQTT_BASE_TOPIC topic
