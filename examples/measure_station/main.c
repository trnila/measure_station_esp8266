#include <FreeRTOS.h>
#include <task.h>
#include <esp/uart.h>
#include "wifi.h"
#include "mqtt.h"
#include "fan.h"

void dht_task(void *args);
void dust_task(void *args);

void user_init(void) {
    uart_set_baud(0, 115200);

    vSemaphoreCreateBinary(wifi_alive);

	fan_init();
	mqtt_init();

    xTaskCreate(dht_task, "dht_task", 256, NULL, 2, NULL);
    xTaskCreate(dust_task, "dust_Task", 256, NULL, 2, NULL);
    xTaskCreate(fan_task, "fan_task",  256, NULL, 2, NULL);
    xTaskCreate(wifi_task, "wifi_task",  256, NULL, 2, NULL);
    xTaskCreate(mqtt_task, "mqtt_task",  768, NULL, 2, NULL);
}
