#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#include "constants.h"

OneWireBus * owb_handle;
DS18B20_Info * ds18b20_device;

// --- WIFI Functions
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

void app_main(void)
{
    // ---------------------------- Setup 1-Wire Bus ----------------------
    vTaskDelay(2000.0 / portTICK_PERIOD_MS);

    owb_rmt_driver_info rmt_driver_info;
    owb_handle = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb_handle, true);  // enable CRC check for ROM code


    // For a single device only:
    OneWireBus_ROMCode rom_code;
    owb_status status = owb_read_rom(owb_handle, &rom_code);
    if (status != OWB_STATUS_OK)
    {
        ESP_LOGE(TAG, "An error occurred reading ROM code: %d", status); while(1) {}
    }
    else{
        char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
        owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
        ESP_LOGI(TAG, "Single device %s present", rom_code_s);
    }
    // Create DS18B20 devices on the 1-Wire bus
    ds18b20_device = ds18b20_malloc();  // heap allocation
    ds18b20_init_solo(ds18b20_device, owb_handle); // only one device on bus
    ds18b20_use_crc(ds18b20_device, true);
    ds18b20_set_resolution(ds18b20_device, SELECTED_DS18B20_RESOLUTION);


    // ---------------------- Setup Wifi -------------------------
    init_esp_now();
    // Set the peer address
    esp_now_peer_info_t peerInfo_clock = {}; // First peer is the nixxie clock
    esp_now_peer_info_t peerInfo_Eink = {}; // Second peer is the e-ink display
    memcpy(peerInfo_clock.peer_addr, clock_display_mac, 6);
    memcpy(peerInfo_Eink.peer_addr, Waveshare_E_Ink_mac, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo_clock));
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo_Eink));

    // --- Main Loop ---
    struct_message temperature_send_payload;
    uint32_t temperature_send_payload_id = 1;
    while (1)
    {
        // Create a message to send
        temperature_send_payload.id = -1; 
        temperature_send_payload.sec_since_last_update = 0;
        temperature_send_payload.temperature_inside = 0.0;
        temperature_send_payload.temperature_outside = 0.0;
        // Read the temperature
        ds18b20_convert_all(owb_handle);
        ds18b20_wait_for_conversion(ds18b20_device);
        if(ds18b20_read_temp(ds18b20_device, &temperature_send_payload.temperature_outside) == DS18B20_OK){
            temperature_send_payload.temperature_outside = roundf(temperature_send_payload.temperature_outside * 10) / 10; // Round to 1 decimal place
            temperature_send_payload.id = temperature_send_payload_id++;
            temperature_send_payload.temperature_inside = temperature_send_payload.temperature_outside; // temporary override
            ESP_LOGI(TAG, "Temperature reading: %.2f", temperature_send_payload.temperature_outside);
            // Send the message
            esp_err_t result = esp_now_send(NULL, (uint8_t *)&temperature_send_payload, sizeof(temperature_send_payload)); // NULL as receiver means send to all peers
            if (result == ESP_OK)
            {
                ESP_LOGI(TAG, "Data sent successfully");
            }
            else
            {
                ESP_LOGE(TAG, "Error sending data: %d", result);
            }
        }
        else{
            ESP_LOGE(TAG, "Error reading temperature");
        }

        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}