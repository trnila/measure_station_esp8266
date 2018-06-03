#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <esp8266.h>
#include <espressif/esp_common.h>
#include <dht/dht.h>
#include "config.h"
#include "mqtt.h"
#include "utils.h"

void dht_task(void *args) {
    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pullup(DHT_PIN, false, false);

    for(;;) {
		int16_t temperature = 0;
		int16_t humidity = 0;

        if (dht_read_data(DHT_SENSOR_TYPE, DHT_PIN, &humidity, &temperature)) {
            debug("Humidity: %d%% Temp: %dC", humidity / 10, temperature / 10);

			publish(MQTT_TOPIC("temperature"), 0, "%d", temperature / 10);
			publish(MQTT_TOPIC("humidity"), 0, "%d", humidity / 10);
        } else {
            debug("Could not read data from sensor");
        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

