#include <stdio.h>
#include <string.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

typedef struct struct_message {
    int id;
    float temperature;
    float humidity;
} struct_message;

struct_message incomingData;

static const char *TAG = "ESP-NOW Receiver";

// Callback when data is received
void on_data_receive(const uint8_t *mac_addr, const uint8_t *data, int len) {
    memcpy(&incomingData, data, sizeof(incomingData));
    ESP_LOGI(TAG, "Received data: ID=%d, Temperature=%.2f, Humidity=%.2f", incomingData.id, incomingData.temperature, incomingData.humidity);
}

void init_esp_now() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_receive));
}

void app_main() {
    init_esp_now();

    // The receiver does not need to add any peers, just waits for the data to be received
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
