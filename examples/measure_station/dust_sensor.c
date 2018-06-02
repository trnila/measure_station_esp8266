#include <FreeRTOS.h>
#include <task.h>
#include <esp8266.h>
#include <espressif/esp_common.h>
#include "config.h"
#include "mqtt.h"

void dust_task(void* args) {
	gpio_enable(DUST_LED_PIN, GPIO_OUTPUT);
	gpio_write(DUST_LED_PIN, 1);

	for(;;) {
		taskENTER_CRITICAL();
		gpio_write(DUST_LED_PIN, 0);
		sdk_os_delay_us(280);
		int pm = sdk_system_adc_read();
		sdk_os_delay_us(40);
		gpio_write(DUST_LED_PIN, 1);
		taskEXIT_CRITICAL();

		int computed = 0.17 * pm - 0.1;
		publish("measure_station/dust", "%d", computed);
		printf("dust %d %d\n", pm, computed);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
