#include <stdarg.h>
#include <string.h>
#include <paho_mqtt_c/MQTTClient.h>
#include <paho_mqtt_c/MQTTESP8266.h>
#include "FreeRTOS.h"
#include "utils.h"
#include <semphr.h>
#include "wifi.h"
#include "config.h"
#include "task.h"


typedef struct {
	char topic[64];
	char payload[64];
} Message;

QueueHandle_t publish_queue;

void mqtt_init() {
    publish_queue = xQueueCreate(3, sizeof(Message));
	if(!publish_queue) {
		panic("could not allocate publish queue");
	}
}

void publish(const char* topic, const char* msg_fmt, ...) {
	va_list va;
	va_start(va, msg_fmt);

	Message msg;

	if(strlen(topic) > sizeof(msg.topic)) {
		panic("Topic too large");
	}

	strcpy(msg.topic, topic);
	vsnprintf(msg.payload, sizeof(msg.payload), msg_fmt, va);

	if(xQueueSend(publish_queue, &msg, portMAX_DELAY) != pdTRUE) {
		panic("could not publish to mqtt queue");
	}

	va_end(va);
}

void  mqtt_task(void *pvParameters) {
    int ret         = 0;
    struct mqtt_network network;
    mqtt_client_t client   = mqtt_client_default;
    char mqtt_client_id[20];
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    mqtt_network_new( &network );
    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, "ESP-");
    strcat(mqtt_client_id, "station");//get_my_id());

    while(1) {
        xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__,
               MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if( ret ){
            printf("connect error: %d\n\r", ret);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                      mqtt_readbuf, 100);

        data.willFlag       = 0;
        data.MQTTVersion    = 4;
        data.clientID.cstring   = mqtt_client_id;
        data.username.cstring   = MQTT_USER;
        data.password.cstring   = MQTT_PASS;
        data.keepAliveInterval  = 10;
        data.cleansession   = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret){
            printf("send mqtt error: %d\n\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        printf("done\r\n");
//        mqtt_subscribe(&client, "/esptopic", MQTT_QOS1, topic_received);
        xQueueReset(publish_queue);

/*
		int i = 0;
	for(;;) {
				const char msg[10];
				snprintf(msg, sizeof(msg), "ahoj%d\n", i++);
                mqtt_message_t message;
                message.payload = msg;
                message.payloadlen = PUB_MSG_LEN;
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = 0;
                ret = mqtt_publish(&client, "/danitest", &message);

        vTaskDelay( 1000 / portTICK_PERIOD_MS );

	}
*/

        while(1){
			Message msg;
            while(xQueueReceive(publish_queue, &msg, 0) == pdTRUE) {
                printf("got message to publish\r\n");
                mqtt_message_t message;
                message.payload = msg.payload;
                message.payloadlen = strlen(msg.payload);
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = 0;
                ret = mqtt_publish(&client, msg.topic, &message);
                if (ret != MQTT_SUCCESS ){
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
