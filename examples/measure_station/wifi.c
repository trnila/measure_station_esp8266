#include <ssid_config.h>
#include <stdint.h>
#include <esp8266.h>
#include <espressif/esp_common.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include "utils.h"

SemaphoreHandle_t wifi_alive;

void wifi_task(void *args) {
    uint8_t status  = 0;
    uint8_t retries = 30;
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    debug("WiFi: connecting to WiFi");
	sdk_wifi_set_sleep_type(WIFI_SLEEP_MODEM);
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    for(;;) {
        while ((status != STATION_GOT_IP) && (retries)) {
            status = sdk_wifi_station_get_connect_status();
            debug("status = %d", status );
            if( status == STATION_WRONG_PASSWORD ){
                debug("WiFi: wrong password");
                break;
            } else if( status == STATION_NO_AP_FOUND ) {
                debug("WiFi: AP not found");
                break;
            } else if( status == STATION_CONNECT_FAIL ) {
                debug("WiFi: connection failed");
                break;
            }
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            --retries;
        }
        if (status == STATION_GOT_IP) {
            debug("WiFi: Connected");
            xSemaphoreGive( wifi_alive );
            taskYIELD();
        }

        while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
            xSemaphoreGive( wifi_alive );
            taskYIELD();
        }
        debug("WiFi: disconnected");
        sdk_wifi_station_disconnect();
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}
