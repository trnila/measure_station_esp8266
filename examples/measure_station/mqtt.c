#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <paho_mqtt_c/MQTTClient.h>
#include <paho_mqtt_c/MQTTESP8266.h>
#include "config.h"
#include "utils.h"
#include "wifi.h"

typedef struct {
	char topic[64];
	char payload[64];
	char retained;
} Message;

typedef struct SubscribedTopic {
	char topic[32];
	void (*callback)(mqtt_message_data_t*);
	struct SubscribedTopic* next;
} SubscribedTopic;

QueueHandle_t publish_queue;
SubscribedTopic *subscribed_topics = NULL;

void mqtt_init() {
    publish_queue = xQueueCreate(3, sizeof(Message));
	if(!publish_queue) {
		panic("could not allocate publish queue");
	}
}

void publish(const char* topic, const int retained, const char* msg_fmt, ...) {
	va_list va;
	va_start(va, msg_fmt);

	Message msg;

	if(strlen(topic) > sizeof(msg.topic)) {
		panic("Topic too large");
	}

	strcpy(msg.topic, topic);
	vsnprintf(msg.payload, sizeof(msg.payload), msg_fmt, va);
	msg.retained = retained;

	if(xQueueSend(publish_queue, &msg, portMAX_DELAY) != pdTRUE) {
		printf("ERROR: could not publish to mqtt queue");
	}

	va_end(va);
}

void subscribe(const char *topic, void (*callback)(mqtt_message_data_t *)) {
	SubscribedTopic *sub = malloc(sizeof(SubscribedTopic));
	if(strlen(topic) > sizeof(sub->topic)) {
		panic("subscribe: topic too large");
	}

	strcpy(sub->topic, topic);
	sub->callback = callback;
	sub->next = NULL;

	if(!subscribed_topics) {
		subscribed_topics = sub;
	} else {
		SubscribedTopic* parent = subscribed_topics;
		while(parent && parent->next) {
			parent = parent->next;
		}

		parent->next = sub;
	}
}

void mqtt_task(void *args) {
    int ret = 0;
    struct mqtt_network network;
    mqtt_client_t client = mqtt_client_default;
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;
    mqtt_network_new( &network );

    for(;;) {
        xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__, MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if(ret) {
            printf("connect error: %d\n\r", ret);
            taskYIELD();
            continue;
        }

        printf("connected to mqtt\n\r");
        mqtt_client_new(
				&client, &network, 5000,
				mqtt_buf, sizeof(mqtt_buf),
				mqtt_readbuf, sizeof(mqtt_readbuf)
		);

        data.willFlag = 0;
        data.MQTTVersion = 4;
        data.clientID.cstring = MQTT_CLIENT_ID;
        data.username.cstring = MQTT_USER;
        data.password.cstring = MQTT_PASS;
        data.keepAliveInterval  = 10;
        data.cleansession = 0;
        printf("Send MQTT connect ... \n\r");
        ret = mqtt_connect(&client, &data);
        if(ret) {
            printf("send mqtt error: %d\n\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        printf("done\r\n");
        xQueueReset(publish_queue);
		
		// register all topics
		SubscribedTopic* topic = subscribed_topics;
		while(topic) {
			printf("subscribe topic %s\n", topic->topic);
			mqtt_subscribe(&client, topic->topic, MQTT_QOS1, topic->callback);
			topic = topic->next; 
		}

        for(;;) {
			Message msg;
            while(xQueueReceive(publish_queue, &msg, 0) == pdTRUE) {
                printf("got message to publish on topic %s\r\n", msg.topic);
                mqtt_message_t message;
                message.payload = msg.payload;
                message.payloadlen = strlen(msg.payload);
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = msg.retained;
                ret = mqtt_publish(&client, msg.topic, &message);
                if(ret != MQTT_SUCCESS ) {
                    printf("error while publishing message: %d\n", ret );
                    break;
                }
            }

            ret = mqtt_yield(&client, 1000);
            if (ret == MQTT_DISCONNECTED)
                break;
        }
        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}
