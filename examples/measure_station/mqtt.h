#pragma once

void mqtt_init();
void publish(const char* topic, const char* msg_fmt, ...);
void  mqtt_task(void *pvParameters);
