#pragma once
#include <FreeRTOS.h>
#include <semphr.h>

void  wifi_task(void *pvParameters);

extern SemaphoreHandle_t wifi_alive;
