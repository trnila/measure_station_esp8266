#include <stdio.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <i2c/i2c.h>
#include <bh1750/bh1750.h>
#include "mqtt.h"
#include "config.h"
#include "utils.h"

void bh1750_task(void *args) {
    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);

    i2c_dev_t dev = {
        .addr = BH1750_ADDR_LO,
        .bus = I2C_BUS,
    };
    bh1750_configure(&dev, BH1750_CONTINUOUS_MODE | BH1750_HIGH_RES_MODE);

    for(;;) {
		int lux = bh1750_read(&dev);
		debug("Lux: %d", lux);
		publish(MQTT_TOPIC("lux"), 0, "%d", lux);

		vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}
