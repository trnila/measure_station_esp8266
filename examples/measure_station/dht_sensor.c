#include <stdio.h>
#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <dht/dht.h>
#include <semphr.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <utils.h>

#include "mqtt.h"
#include "wifi.h"


/* An example using the ubiquitous DHT** humidity sensors
 * to read and print a new temperature and humidity measurement
 * from a sensor attached to GPIO pin 4.
 */
uint8_t const dht_gpio = 4;
const dht_sensor_type_t sensor_type = DHT_TYPE_DHT22;

void dhtMeasurementTask(void *pvParameters)
{
    int16_t temperature = 0;
    int16_t humidity = 0;

    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pullup(dht_gpio, false, false);

    while(1) {
        if (dht_read_data(sensor_type, dht_gpio, &humidity, &temperature)) {
            printf("Humidity: %d%% Temp: %dC\n", 
                    humidity / 10, 
                    temperature / 10);

			publish("measure_station/temperature", "%d", temperature / 10);
			publish("measure_station/humidity", "%d", humidity / 10);
        } else {
            printf("Could not read data from sensor\n");
        }

        // Three second delay...
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

int sharp_led_pin = 5;

int counter = 0;
int pm;

/*
void frc1_interrupt_handler(void *arg) {
	counter++;

	if(counter == 28/4) {
		pm = sdk_system_adc_read();
	} else if(counter == 32/4) {
		gpio_write(sharp_led_pin, 1);
	} else if(counter >= 1000000/4) {
		gpio_write(sharp_led_pin, 0);
		counter = 0;
	}
}
*/

void dustTask(void* args) {
	gpio_enable(sharp_led_pin, GPIO_OUTPUT);
	gpio_write(sharp_led_pin, 1);

	/*
	timer_set_interrupts(FRC1, false);
	timer_set_run(FRC1, false);

	_xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler, NULL);
	 timer_set_frequency(FRC1, 250000);

	timer_set_interrupts(FRC1, true);
    timer_set_run(FRC1, true);
*/


	for(;;) {
//		printf("ADC voltage is %.3f\n", 1.0 / 1024 * sdk_system_adc_read());



		taskENTER_CRITICAL();
		gpio_write(sharp_led_pin, 0);
		sdk_os_delay_us(280);
		pm = sdk_system_adc_read();
		sdk_os_delay_us(40);
		gpio_write(sharp_led_pin, 1);
		taskEXIT_CRITICAL();


		int my_pm = pm;
		int computed = 0.17 * my_pm - 0.1;

		publish("measure_station/dust", "%d", computed);

		printf("PM %d %f\n", my_pm, computed);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}



void user_init(void) {
    uart_set_baud(0, 115200);

    vSemaphoreCreateBinary(wifi_alive);

	mqtt_init();

    xTaskCreate(dhtMeasurementTask, "dhtMeasurementTask", 256, NULL, 2, NULL);
    xTaskCreate(dustTask, "dustTask", 256, NULL, 2, NULL);
    xTaskCreate(wifi_task, "wifi_task",  256, NULL, 2, NULL);
    xTaskCreate(mqtt_task, "mqtt_task",  768, NULL, 2, NULL);
}

