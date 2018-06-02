#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <esp8266.h>
#include <espressif/esp_common.h>
#include <dht/dht.h>
#include "config.h"
#include "mqtt.h"

void dht_task(void *args) {
    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pullup(DHT_PIN, false, false);

    for(;;) {
		int16_t temperature = 0;
		int16_t humidity = 0;

        if (dht_read_data(DHT_SENSOR_TYPE, DHT_PIN, &humidity, &temperature)) {
            printf("Humidity: %d%% Temp: %dC\n", humidity / 10, temperature / 10);

			publish("measure_station/temperature", "%d", temperature / 10);
			publish("measure_station/humidity", "%d", humidity / 10);
        } else {
            printf("Could not read data from sensor\n");
        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

