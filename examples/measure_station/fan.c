#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "utils.h"
#include "mqtt.h"

static QueueHandle_t fan_queue;
static int fan_state = 0;

static int cmp(const char* a, const char *b, int len) {
	while(len-- && *a && *b) {
		char ca = isupper(*a) ? (*a) + 'a' - 'A' : (*a);
		char cb = isupper(*b) ? (*b) + 'a' - 'A' : (*b);

		if(ca != cb) {
			return ca - cb;
		}

		a++;
		b++;
	}

	return 0;
}


static void topic_received(mqtt_message_data_t *md) {
	mqtt_message_t *m = md->message;

	char state = cmp(m->payload, "on", m->payloadlen) == 0  ||
			cmp(m->payload, "1", m->payloadlen) == 0 ||
			cmp(m->payload, "true", m->payloadlen) == 0;

	xQueueSend(fan_queue, &state, 0);
}

void fan_init() {
    fan_queue = xQueueCreate(3, sizeof(char));
	if(!fan_queue) {
		panic("could not allocate queue");
	}

	gpio_enable(FAN_PIN, GPIO_OUTPUT);
	gpio_write(FAN_PIN, 0);
	subscribe(MQTT_TOPIC("fan/set"), topic_received);
	subscribe(MQTT_TOPIC("fan/state"), topic_received);
}


void fan_task(void* arg) {
	for(;;) {
		char state;
		xQueueReceive(fan_queue, &state, portMAX_DELAY);

		printf("fan set state to %d\n", state);
		gpio_write(FAN_PIN, state);

		if(fan_state != state) {
			publish(MQTT_TOPIC("fan/state"), 1, state ? "ON" : "OFF");
		}
		fan_state = state;
	}
}
