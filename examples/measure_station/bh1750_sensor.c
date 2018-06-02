#include <stdio.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <i2c/i2c.h>
#include <bh1750/bh1750.h>
#include "mqtt.h"
#include "config.h"

void bh1750_task(void *args) {
    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);

    i2c_dev_t dev = {
        .addr = BH1750_ADDR_LO,
        .bus = I2C_BUS,
    };
    bh1750_configure(&dev, BH1750_CONTINUOUS_MODE | BH1750_HIGH_RES_MODE);

    for(;;) {
		int lux = bh1750_read(&dev);
		printf("Lux: %d\n", lux);
		publish(MQTT_TOPIC("lux"), 0, "%d", lux);

		vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
