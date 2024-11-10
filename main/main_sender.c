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

// --- ESP-NOW sender ---

// MAC address of the receiver
uint8_t receiver_mac[] = {0x24, 0x6f, 0x28, 0x24, 0x75, 0x1c}; 

typedef struct struct_message {
    int id;
    int sec_since_last_update;
    float temperature_outside;
    float temperature_inside;
} struct_message;

struct_message myData;

static const char *TAG = "ESP-NOW Sender";

// Callback when data is sent
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGI(TAG, "Last Packet Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_data_sent));
}

void app_main(void) {
    init_esp_now();

    // Set the peer address
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiver_mac, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));

    // Prepare the data
    myData.id = 1;
    myData.sec_since_last_update = 0;
    myData.temperature_outside = 70.2;
    myData.temperature_inside = 30.2;

    // Send data every 2 seconds
    while (true) {
        esp_err_t result = esp_now_send(receiver_mac, (uint8_t *) &myData, sizeof(myData));
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Data sent successfully");
        } else {
            ESP_LOGE(TAG, "Error sending data: %d", result);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
